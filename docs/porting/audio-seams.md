# Audio seams (DirectSound / `mmio*` / `wave_stream`)

- **Issue**: [#68](https://github.com/lollipop-onl/railsim2-portable/issues/68) (parent [#7](https://github.com/lollipop-onl/railsim2-portable/issues/7))
- **Graphics ADR (audio row)**: [adr-backend.md](adr-backend.md)
- **Tree**: game `*.cpp` / `lib/*.cpp` at this document's commit. Counts exclude comments unless noted.
- **This slice does not implement OpenAL / miniaudio / SDL_mixer or change `CSoundEffector` / `CWaveArray` behavior.** Later `#7` slices should replace **this closed set**, not walk the whole tree.

## Corrections to issue #68 / parent #7

| Claim | Fact |
|-------|------|
| `lib/sound.cpp` / `lib/wave.cpp` / `lib/wave_stream.cpp` depend on DirectSound buffers / 3D / listener | True. All live COM calls sit in those three TUs plus inlines in `lib/wave.h` / `lib/sound.h`. |
| WAV load is `mmio*` (winmm); assets are 16 `.wav` files | True. Eight unique files, copied in `Distribution/en` and `Distribution/jp`. See [Shipped WAV set](#shipped-wav-set). |
| Inventory `waveIn*` and other winmm symbols | **`waveIn*` / `waveOut*` have zero callers.** Live winmm audio is **`mmio*` only**, in `CWave::Load` / `CreateBuffer`. `timeGetTime` is a clock seam, not playback. |
| `lib/wave_stream.h` is the only `CreateThread` / `_beginthreadex` user (web-thread argument) | **Half true, and not an audio thread.** `CWaveStream` never starts a thread. `CThread` (`CreateThread`) has **zero** callers. Live `_beginthreadex` is `CCrtThread` used by **`lib/input.cpp`** (input poll; parent `#8`). |
| M4 exit is 3D running-sound localization | True for **rail wheel** (`CWaveArray` with `f3d=true`) and **plugin `SoundEffect`** (`CSoundEffector::SetPos`). Skin UI waves load with `f3d=false`. |
| `CWaveStream` must be redesigned before M4 | **No live callers.** `docs/porting/api-surface.md` already lists it as unused from game. Do not block `#7` playback on a streamer that nothing enqueues. |

`lib/music.cpp` (DirectMusic + `IDirectSound` QI) is commented out of `lib/udx.h` and `InitDirectMusic` is commented in `lib/main.cpp`. Out of this set (`#16` / media).

## Architecture

```
DirectSound8 / primary / listener          lib/sound.cpp (InitDirectSound, SetListener*)
        |                                          |
        v                                          v
   svs.pDS / pPB / pListener / f3D          CWave (lib/wave.cpp)  <-- mmio* PCM load
                                                    |
                    +-------------------------------+----------------------------+
                    |                               |                            |
              CWaveArray                      CSoundEffector               CWaveStream
           (skin UI 2D,                   (plugin SoundEffect,            (notify ring;
            rail wheel 3D)                 3D loop / one-shot)             zero callers)
```

Game code never includes `<dsound.h>` directly. `lib/headers.h` pulls `<dmusici.h>` which includes `port/stub/dsound.h`, and typedefs `LPSNDBUF` / `LP3DBUF` / `LP3DLISTENER`. The **public seam** for `#7` is `lib/sound.h` + `lib/wave.h` (and the two game wrappers below), not raw DirectSound types.

Init failure is soft: `DirectSoundCreate8` fail sets `svs.pDS = NULL` and still returns `TRUE`. Later `CWave` loads no-op. `CConfigMode::CheckHardware` then offers `DirectSoundProblem` if any sound checkbox is on.

`/3ds` forces `svs.f3D = FALSE`. `/fx` is parsed but `svs.fFX` is **hardcoded `FALSE`**.

## Closed DirectSound set

All COM calls live in **`lib/sound.cpp`**, **`lib/wave.cpp`**, **`lib/wave.h`** (inlines), or unused **`lib/wave_stream.cpp`**.

### Device / primary / listener (`lib/sound.cpp`)

| API / symbol | Function | Role |
|--------------|----------|------|
| `DirectSoundCreate8` | `InitDirectSound` | `svs.pDS`; fail -> `NULL`, return `TRUE` |
| `IDirectSound8::SetCooperativeLevel` | `InitDirectSound` | `svw.hWnd`, `DSSCL_PRIORITY` |
| `IDirectSound8::CreateSoundBuffer` | `CreatePrimaryBuffer` | primary: `DSBCAPS_CTRLVOLUME \| DSBCAPS_PRIMARYBUFFER` (+ `DSBCAPS_CTRL3D` if `f3D`) |
| `IDirectSoundBuffer::SetFormat` | `CreatePrimaryBuffer` | `GetWaveFormat(44100, 16, 2)` PCM |
| `IDirectSoundBuffer::GetVolume` / `SetVolume` | create / `SetMasterVolume` | hundredths of a dB; `DSBVOLUME_MIN`..`DSBVOLUME_MAX` |
| `IDirectSoundBuffer::GetStatus` / `Restore` | `PrimaryBufferVerify` | `DSBSTATUS_BUFFERLOST` |
| `QueryInterface(IID_IDirectSound3DListener)` | `CreatePrimaryBuffer` | `svs.pListener` when `f3D` |
| `IDirectSound3DListener::SetDistanceFactor` | `SetListenerSens` | init `1.0`; `Main()` then sets **`10.0`** |
| `IDirectSound3DListener::SetPosition` | `SetListenerPos` | meters, `DS3D_IMMEDIATE` |
| `IDirectSound3DListener::SetOrientation` | `SetListenerDir` | forward + up, `DS3D_IMMEDIATE` |

`SetMasterVolume()` with no args restores `svs.initVolume`. `CGameMode::Spin` calls it every loop; inactive-window path calls it again.

### Secondary buffer / 3D source (`lib/wave.cpp`, `lib/wave.h`)

| API / symbol | Function | Role |
|--------------|----------|------|
| `IDirectSound8::CreateSoundBuffer` | `CWave::CreateBuffer` | `DSBCAPS_CTRLVOLUME` (+ `CTRL3D` if `svs.f3D`); size = `data` chunk |
| `IDirectSoundBuffer::Lock` / `Unlock` | `CreateBuffer` | fill from `mmioRead`; `DSERR_BUFFERLOST` -> `Restore` and fail |
| `QueryInterface(IID_IDirectSound3DBuffer)` | `CWave::Query` | `m_p3D` when `f3D` |
| `QueryInterface(IID_IDirectSoundBuffer8)` | `CWave::Query` | `m_pFX`; only if `svs.fFX` (**always false today**) |
| `Play` / `Stop` / `SetCurrentPosition` | `CWave::Play` / `Stop` | `ms < 0` -> `DSBPLAY_LOOPING` from 0; else seek `m_BytesPerSec * ms / 1000` |
| `GetStatus` | `CWave::GetStatus` | `DSBSTATUS_PLAYING` |
| `SetVolume` | `CWave::SetVolume` | same dB units as master |
| `IDirectSound3DBuffer::SetPosition` | `CWave::SetPos` | meters, `DS3D_IMMEDIATE` |
| `IDirectSound3DBuffer::SetVelocity` | `CWave::SetVelocity` | **no callers** |
| `IDirectSoundBuffer8::SetFX` | `CWave::SetFX` | **no game callers**; `fFX` off |
| `DuplicateSoundBuffer` | `CWave::Duplicate` | **dead**: `CWaveArray::Load` has the duplicate path under `#if 0` and loads N copies instead |
| `Restore` + reload | `CWave::Play` | `DSERR_BUFFERLOST` -> `PrimaryBufferVerify`, `Restore`, `Load(m_strName)` |

`DSERR_BUFFERTOOSMALL` on create is treated as success with no buffer (short clip / FX comment). That path is leftover from the unused FX flag.

### Stream notify (`lib/wave_stream.cpp`, unused)

| API / symbol | Function | Role |
|--------------|----------|------|
| `CreateSoundBuffer` | `CWaveStream::Begin` | `DSBCAPS_LOCDEFER \| CTRLPOSITIONNOTIFY \| GETCURRENTPOSITION2` |
| `Lock` / `Unlock` / `Play(DSBPLAY_LOOPING)` / `Stop` | `Begin` / `End` / `Enqueue` | ring of `size * count` bytes |
| `QueryInterface(IID_IDirectSoundNotify)` | `Begin` | `m_pNotify` (result of QI is **not** assigned to `hr` before the `FAILED(hr)` check) |
| `SetNotificationPositions` | `Begin` | one event at `size*i + size - 1` for each block |
| `GetCurrentPosition` | `Enqueue` | clamp write cursor out of the play/write hazard |
| `CreateEvent` / `WaitForSingleObject` | `Begin` / `Enqueue` | `STREAM_TIMEOUT` = `INFINITE` |

No game or `lib/` TU constructs `CWaveStream`. Do not grow stubs for notify until a caller appears.

### Listener / master callers

| Function | File | When |
|----------|------|------|
| `InitDirectSound` / `FreeDirectSound` | `lib/main.cpp` | `CApp` ctor / dtor (`#ifndef NO_SOUNDS`) |
| `SetListenerSens(10.0f)` / `SetMasterVolume()` | `RailSim2.cpp` `Main()` | once after startup |
| `SetMasterVolume()` | `CGameMode::Spin` | every loop (also when inactive) |
| `SetListenerPos(GetVPos())` / `SetListenerDir(GetVDir(), GetVUp())` | `CGameMode::SpinSound` | camera / view pose |
| `SpinSound` | `CSceneryMode` (4 sites), `CInterfaceMode` | frame audio sync |

`SpinSound` also writes `g_FrameDelta` from `HighTimer()` for wheel-sound delay. That timer is not a DirectSound API.

## Closed `mmio*` set (WAV parser)

**One file:** `lib/wave.cpp`. `<mmsystem.h>` is included from `lib/headers.h` for every udx TU; game sources never call `mmio*`.

| API | Function | Role |
|-----|----------|------|
| `mmioOpen(..., MMIO_READ \| MMIO_ALLOCBUF)` | `CWave::Load` | open `strFile` (cwd / virtual cwd; `#4` / `rs2_fopen` does not wrap mmio) |
| `mmioDescend(..., MMIO_FINDRIFF)` | `Load` | require `mmioFOURCC('W','A','V','E')` |
| `mmioDescend` (flags `0`) | `Load` | child `fmt ` under that RIFF |
| `mmioRead` | `Load` | `sizeof(WAVEFORMATEX)` into `wfmtx` |
| `mmioAscend` | `Load` | leave `fmt ` |
| `mmioDescend(..., MMIO_FINDCHUNK)` | `Load` | child `data` |
| `mmioRead` | `CreateBuffer` | PCM bytes into the locked DirectSound buffer (possibly two segments) |
| `mmioClose` | `Load` | success and error paths |

`Capture.cpp` uses `mmioFOURCC('D','I','B',' ')` for an AVI video stream (`#17`). Its `WAVEFORMATEX` audio-mux block is `#if 0`. Not this set.

### Minimum fields a replacement parser must honor

`CWave::Load` rejects anything that is not **PCM** (`wFormatTag == WAVE_FORMAT_PCM`). It does not read `fact`, extra `fmt` bytes (`cbSize`), or extra chunks after `data`.

| Field | Why it is required |
|-------|--------------------|
| RIFF form type `WAVE` | `MMIO_FINDRIFF` |
| `fmt ` `wFormatTag` | must be `WAVE_FORMAT_PCM` (1) |
| `fmt ` `nAvgBytesPerSec` | stored as `m_BytesPerSec`; `Play(ms)` seeks with it |
| `fmt ` `nChannels`, `nSamplesPerSec`, `wBitsPerSample`, `nBlockAlign` | passed through to `CreateSoundBuffer` / `lpwfxFormat` |
| `data` `cksize` | secondary-buffer byte length |
| `data` payload | raw PCM, no conversion |

Primary mix format is **44.1 kHz / 16-bit / stereo**. Files may differ; DirectSound resamples. Shipped assets are **22.05 kHz / 8-bit / mono** PCM (see below). A portable parser can keep PCM as-is and let the backend convert.

`CWave::Load` still calls `_fullpath` for the reload cache key (`m_strName`). That is the `#4` path seam, already joined against the virtual cwd.

## `wave_stream` thread / buffer contract (short)

`CWaveStream` is a **notify-driven ring**, not a worker thread:

1. **`Begin(format, size, count)`** ? create a looping secondary buffer of `size * count` bytes; arm `count` notify offsets; `Play(..., DSBPLAY_LOOPING)`. Fails if `svs.pDS` is null.
2. **`Enqueue(pData, size)`** ? block on the shared notify event (`INFINITE`); optionally snap `m_wpos` to the hardware write cursor if it sits in the play/write hazard; `Lock` `size` bytes at `m_wpos`; copy; wrap `m_wpos`. `size` must equal the `Begin` block size.
3. **`End()`** ? `Stop`, `RELEASE` buffer + notify, `CloseHandle` the event.

There is **no producer thread inside `CWaveStream`**. A caller would have to feed `Enqueue` (from the main thread or its own thread). **No such caller exists.**

`CThread` / `CCrtThread` in the same header:

| Class | Start API | Live audio use | Live use elsewhere |
|-------|-----------|----------------|--------------------|
| `CThread` | `CreateThread` | none | none |
| `CCrtThread` | `_beginthreadex` | none | `lib/input.cpp` `g_InputPollingThread` (`#8`) |

`CWaveArray::AddLoop` is **declared and never defined**; zero callers. Ignore it.

**`#7` / web:** M4 playable does not need a streamer. If a later slice revives music or long loops via `CWaveStream`, feed the backend from a **mix / queue callback** (OpenAL buffer queue, miniaudio data callback). Do not add a `CreateThread` just to copy this unused class. SharedArrayBuffer is not implied by current audio.

## Public play API (backend must preserve)

### `lib/sound.h`

| Function | DirectSound source | Notes |
|----------|-------------------|-------|
| `InitDirectSound` / `FreeDirectSound` | device + primary + listener | keep the soft-fail `pDS == NULL` behavior |
| `SetMasterVolume(dB)` | primary `SetVolume` | hundredths of a dB |
| `SetListenerSens` / `SetListenerPos` / `SetListenerDir` | 3D listener | no-op when `!svs.f3D` |

### `lib/wave.h` (`CWave`)

| Function | Notes |
|----------|-------|
| `Load(path)` | PCM WAV via the mmio contract; `_fullpath` cache key |
| `Play(ms)` | `ms < 0` loop; else start offset in ms |
| `Stop` / `GetStatus` / `SetVolume` / `SetPos` | voice control + 3D position |
| `SetVelocity` / `SetFX` / `Duplicate` | keep signatures; no live game callers |

### Game wrappers (do not rewrite in the first backend PR)

| Wrapper | 3D? | Voices | Trigger |
|---------|-----|--------|---------|
| `CWaveArray::Load(file, n, f3d)` | `f3d` temporarily overrides `svs.f3D` | `n` independent `CWave`s | skin / rail |
| `CWaveArray::Add(pos, vol, ms=0)` | `SetPos` then first idle voice | first `!GetStatus()` | UI / wheel |
| `CSoundEffector::LoadData` / `PlayWave` / `SetPos` | always `CWave::SetPos` (needs `f3D`) | one `CWave` per `CSoundState` | plugin `SoundEffect` |

`CWaveArray` / `CSoundEffector` bodies stay as-is for this inventory. `#7` may keep them and only replace `CWave` / `svs`.

## Who plays what (M4)

Config checkboxes (`CConfigMode`): `GetInterfaceSound`, `GetRailSound`, `GetTrainSound`, `GetStructSound`, `GetSurfaceSound`. `CheckHardware` clears them all if `!svs.pDS`.

| Source | Load | Play | Gate | 3D |
|--------|------|------|------|----|
| Skin UI (6 names) | `CSkinPlugin::Load` -> `CWaveArray` `n=4/2/1`, `f3d=false` | `MouseDown` / `MouseUp` / `Error` / `ScreenShot` / `VideoStart` / `VideoStop` at `V3ZERO`, vol `-1500` | `GetInterfaceSound` | no |
| Rail joint | `CRailPlugin::Load` `WheelSoundFile`, `n=10`, `f3d=true` | `CAxlePosture::Rotate` -> `PlayWheelSound` when axle `m_WheelSound` and scene is `g_Scene` | `GetRailSound`; skip if sim speed `> 1` | yes, axle `m_Pos` |
| Plugin `SoundEffect` | `CModelPlugin::LoadSoundWave` -> `CSoundEffector::LoadData` | `SimulateEffect` -> `CSoundState::Confirm` -> `PlayWave` (`ms = -1` if `Loop`) | `IsSoundEnabled()` = train / struct / surface config | yes, object pose + `SourceCoord` |

Wheel timing: help text requires **100 ms of leading silence** in `Wheel.wav`. `PlayWheelSound` starts at `ms = 100 - delay` (`WHEEL_SOUND_MARGIN`) so the click lands on the joint. A backend that ignores the start-offset argument will smear joints.

Shipped `SoundEffect` example: station `Signal.wav` (`WaveFileName` in `Station2.txt`). Third-party plugins may add more PCM files; the parser contract above is the gate, not the 16-file list.

## Shipped WAV set

`find Distribution -iname '*.wav'` = **16 files** (8 unique Å~ `en`/`jp`). All are PCM **1 ch / 22050 Hz / 8-bit**.

| Relative path (under `Distribution/*/RailSim2/`) | Role | Duration |
|--------------------------------------------------|------|----------|
| `Rail/Default_JR_Narrow/Wheel.wav` | rail 3D (`Default_JR_Standard` points here too) | 0.161 s |
| `Skin/Default_Blue/MouseDown.wav` | UI | 0.200 s |
| `Skin/Default_Blue/MouseUp.wav` | UI | 0.200 s |
| `Skin/Default_Blue/Error.wav` | UI | 0.150 s |
| `Skin/Default_Blue/ScreenShot.wav` | UI | 0.153 s |
| `Skin/Default_Blue/VideoStart.wav` | UI | 0.150 s |
| `Skin/Default_Blue/VideoStop.wav` | UI | 0.150 s |
| `Station/SingleCrossing/Signal.wav` | `SoundEffect` (DoubleCrossing uses `..\SingleCrossing\Signal.wav`) | 0.500 s |

Help (`pi_sym_skin_sound_info.html`, `pi_sym_sound_effector.html`) warns that clips shorter than ~100?200 ms may fail to play ? that matches the leftover `DSERR_BUFFERTOOSMALL` comment, not a second format.

## Stub status

`port/stub/dsound.h` is a compile firewall, not a player. Missing vs this closed set (grow only when `lib/sound.cpp` / `lib/wave.cpp` are allowlisted):

- `DirectSoundCreate8`, `DuplicateSoundBuffer`
- `IDirectSoundBuffer::{SetFormat,GetVolume,Restore,SetCurrentPosition,GetCurrentPosition,Lock}` already partly present; add `Restore`, `SetFormat`, `GetVolume`, `SetCurrentPosition`, `GetCurrentPosition` if still absent
- `IDirectSound3DListener::SetDistanceFactor`
- `IDirectSoundNotify::SetNotificationPositions` / `DSBPOSITIONNOTIFY` (only if stream is revived)
- `DSBUFFERDESC`, `DSBCAPS_*`, `DSSCL_PRIORITY`, `DSBPLAY_LOOPING`, `DSBSTATUS_*`, `DSERR_*`, `IID_IDirectSound*`
- `IDirectSoundBuffer8::SetFX` / `DSEFFECTDESC` (dead)

`port/stub/mmsystem.h` has `WAVEFORMATEX` and `timeGetTime` only. **`mmioOpen` / `Descend` / `Ascend` / `Read` / `Close` / `MMCKINFO` / `mmioFOURCC` are absent.** A parser slice should not add real mmio to the stub; put a small PCM reader in `lib/` or `port/` and keep `CWave::Load` as the single caller.

`lib/sound.cpp` / `lib/wave.cpp` / `lib/wave_stream.cpp` are **not** in `port/native_sources.txt`. This document does not add them.

## ADR: 3D audio backend

- **Status**: accepted (confirms the audio row of [adr-backend.md](adr-backend.md))
- **Issue**: [#68](https://github.com/lollipop-onl/railsim2-portable/issues/68) / parent [#7](https://github.com/lollipop-onl/railsim2-portable/issues/7)
- **Date**: 2026-09-07

### Decision

Use **OpenAL Soft** on native (macOS / Linux / later Windows-from-this-port) and **Emscripten's OpenAL** on web. Map:

| RailSim2 | OpenAL |
|----------|--------|
| `svs.pListener` + `SetListenerPos` / `Dir` / `Sens` | one `AL_LISTENER` (`AL_POSITION`, `AL_ORIENTATION`, `AL_METERS_PER_UNIT` or equivalent distance scale) |
| each `CWave` with `m_p3D` | one `alSource` (`AL_POSITION`, optional `AL_LOOPING`) |
| each `CWave` with `f3d=false` | source with `AL_SOURCE_RELATIVE` at the listener, or a 2D / no-attenuation path |
| `SetVolume` / master (hundredths of a dB) | `AL_GAIN` (convert from dB; primary `SetMasterVolume` can be a listener gain) |
| `Play(ms)` seek | source offset in seconds (`ms/1000`), not bytes |
| PCM from the [mmio contract](#minimum-fields-a-replacement-parser-must-honor) | `alBuffer` (`AL_FORMAT_MONO8` for shipped assets; also `MONO16` / `STEREO*` if a plugin ships them) |

Do **not** link OpenAL in the `check` preset. Same split as the graphics ADR: stubs until the first sound TU is allowlisted.

### Why not the other candidates

| Option | 3D (M4 must-have) | Web | Deps | Verdict |
|--------|-------------------|-----|------|---------|
| **OpenAL Soft** | Listener + sources + distance; matches `SetPos` / `SetListener*` | Emscripten already exposes OpenAL | `find_package` / pkg-config; already named in [adr-backend.md](adr-backend.md) | **Choose** |
| **miniaudio** | `ma_engine` / spatializer can do 3D, but it is a new scene graph next to SDL | Possible via Web Audio backend; not the stack already picked for GL | Header-only, extra model | Fine later if OpenAL Soft packaging fails; not the first implementation |
| **SDL_mixer** / raw SDL2 audio | Mixer / callback. No listener, no per-source position | SDL2 is already the window layer | Zero extra if we abused SDL | **Rejected** in adr-backend: `CWave::SetPos` is a 3D scene |
| **Web Audio API only** | PannerNode is 3D | Excellent on web | Native would be a second stack | Rejected for the same reason as Metal-vs-WebGL |

### Consequences

**`#7` will**

1. Replace `mmio*` inside `CWave::Load` with a PCM reader that honors the field table above.
2. Implement `InitDirectSound` / `CWave` / listener helpers behind the existing names (or a thin `lib/` backend those names call).
3. Preserve `Play(ms)` start-offset semantics for wheel joints.
4. Leave `CWaveStream` unimplemented until something enqueues it; if revived, use an OpenAL buffer queue / callback, not `CreateThread`.

**`#7` will not** (this inventory)

- Link OpenAL / miniaudio / SDL_mixer in this PR.
- Edit `CSoundEffector.cpp` / `CWaveArray.cpp` behavior.
- Touch Capture / AVI (`#17`), DirectMusic, or input's `CCrtThread` (`#8`).

## Out of scope (this slice / `#68`)

- OpenAL Soft (or any backend) implementation or CMake `find_package`
- Adding `lib/sound.cpp` / `lib/wave.cpp` to `port/native_sources.txt`
- `CSoundEffector` / `CWaveArray` logic changes
- Rendering (`#5`), input (`#8`), IME (`#9`), Capture (`#17`)
- `docs/porting/input-seams.md`, `charset-seams.md`, `ffp-render-states.md`
- Issues [#69](https://github.com/lollipop-onl/railsim2-portable/issues/69) / [#70](https://github.com/lollipop-onl/railsim2-portable/issues/70)

## `#7` backend contract (summary)

Replace **`lib/sound.cpp` + `lib/wave.cpp`** (and grow **`port/stub/dsound.h`** / drop `mmio*` as needed) while keeping:

1. **`InitDirectSound` / `FreeDirectSound`** in `CApp` unchanged.
2. **`sound.h` listener + master volume** and **`CWave` load / play / stop / volume / 3D pos**.
3. **PCM-only WAV** with the `fmt ` + `data` fields above; shipped 8-bit mono 22050 Hz plus any plugin PCM.
4. **`Play(ms)`** byte-or-time seek equivalent, including wheel 100 ms lead-in.
5. **`CWaveArray` / `CSoundEffector` call sites** compiling without a rewrite.
6. **`CWaveStream`** left unused (or callback-fed later); do not invent an audio thread for M4.

After `#7`, a reader should be able to answer: "swap these functions and parse these WAV fields, and M4 wheel / UI / plugin sounds still compile and localize."

## Verification

Closed-set DirectSound / mmio / stream symbols should stay inside the files named above (`port/stub/` excepted):

```bash
rg -n --glob '!build/**' --glob '!.git/**' --glob '!Distribution/**' --glob '!port/stub/**' \
  --glob '!docs/**' --glob '!port/rs2_roundtrip*' \
  'DirectSound|CreateSoundBuffer|LPDIRECTSOUND|mmioOpen|mmioRead|mmioDescend|mmioClose|waveIn|waveOut|CWaveStream|SetListener'
```

Expect: `lib/sound.cpp` / `sound.h`, `lib/wave.cpp` / `wave.h`, `lib/wave_stream.cpp` / `wave_stream.h`, `lib/headers.h` typedefs, `lib/main.cpp` init, `RailSim2.cpp` / `CGameMode.cpp` listener, `CConfigMode.cpp` `svs.pDS` check, `CWaveArray` / `CSoundEffector` / skin / rail wrappers, and dead `lib/music.cpp`. No `waveIn*` / `waveOut*`.

`./scripts/check.sh` must stay green. `rs2_wav_pcm_self_test` / `rs2_wav_pcm_distribution` cover the PCM field table (#81).

## Port PCM parser (`port/wav_pcm`)

- **Issue**: [#81](https://github.com/lollipop-onl/railsim2-portable/issues/81) (parent [#7](https://github.com/lollipop-onl/railsim2-portable/issues/7))
- **Entry**: `rs2_wav_pcm_parse` / `rs2_wav_pcm_parse_file` in `port/wav_pcm.h`
- **Result**: `Rs2WavPcm` holds `wFormatTag`, `nChannels`, `nSamplesPerSec`, `nAvgBytesPerSec`, `nBlockAlign`, `wBitsPerSample`, and the raw `data` payload

This is the portable stand-in for the [mmio* contract](#closed-mmio-set-wav-parser). It requires RIFF form `WAVE`, accepts only `wFormatTag == WAVE_FORMAT_PCM` (1), and copies `fmt ` + `data` fields from the [minimum field table](#minimum-fields-a-replacement-parser-must-honor). Other chunks (`fact`, `LIST`, pad bytes) are skipped; extra `fmt` bytes (`cbSize`) are ignored. Non-PCM, truncated RIFF, missing `fmt` / `data`, or a short `data` payload fail with `bool` + reason string.

`CWave::Load` now calls `rs2_wav_pcm_parse_file` (#86). `lib/wave.cpp` is on the allowlist. Do not link OpenAL in the `check` preset.


## CWave::Load uses port/wav_pcm (#86)

- **Issue**: [#86](https://github.com/lollipop-onl/railsim2-portable/issues/86) (parent [#7](https://github.com/lollipop-onl/railsim2-portable/issues/7))
- **Entry**: `CWave::Load` in `lib/wave.cpp` calls `rs2_wav_pcm_parse_file`
- **Allowlist**: `lib/wave.cpp` is in `port/native_sources.txt`

`mmioOpen` / `mmioDescend` / `mmioRead` / `mmioAscend` / `mmioClose` are gone from `Load` and `CreateBuffer`. Non-PCM, broken RIFF, missing `fmt ` / `data`, or a file that cannot be opened still return `FALSE`. On a successful parse, `Rs2WavPcm` maps to `m_BytesPerSec` (`nAvgBytesPerSec`), `m_nChannels`, `m_wBitsPerSample`, and `m_pcm` (raw `data` payload). `CreateBuffer` still fills the DirectSound secondary buffer with `Lock` / `Unlock` from that payload; `lib/sound.cpp` and `port/stub/dsound.h` COM are unchanged. `rs2_wave_load_self_test` / `rs2_wave_load_distribution` check the mapping against the #81 field table. Do not link OpenAL in the `check` preset.

## Port audio stub backend (`port/rs2_audio`)

- **Issue**: [#93](https://github.com/lollipop-onl/railsim2-portable/issues/93) (parent [#7](https://github.com/lollipop-onl/railsim2-portable/issues/7))
- **Entry**: `rs2_audio_intern` / `rs2_audio_play` / `rs2_audio_stop` / `rs2_audio_set_volume` / `rs2_audio_set_pos` / `rs2_audio_set_listener_*` in `port/rs2_audio.h`
- **Backend hooks**: `rs2_audio_backend_*` (stub in `port/rs2_audio.cpp`; OpenAL later replaces intern upload + play/stop/volume/3D/listener)

`CWave::Load` already yields `Rs2WavPcm` (#86). This slice interns that payload as an opaque buffer handle. Same format + bytes return the same handle. Non-PCM tag, empty payload, zero channels/rate, or 8/16-bit mismatch fail. Play records the handle on `rs2_audio_stub_last()`; stop / volume (hundredths of a dB) / 3D position (meters) overwrite that record. Listener pos / dir / distance factor live on `rs2_audio_stub_listener()`.

The check preset links the stub only. There is no OpenAL, no DirectSound COM, and no `lib/sound.cpp` allowlist. `CreateBuffer` / `CWave::Play` still talk to the dsound stub until the next `#7` slice maps `CWave` onto these handles.

| DirectSound / `CWave` | `port/rs2_audio` | Later OpenAL |
|-----------------------|------------------|--------------|
| `CreateSoundBuffer` + `Lock` PCM | `rs2_audio_intern` | `alBuffer` from interned PCM |
| `Play(ms)` / `Stop` | `rs2_audio_play` / `rs2_audio_stop` | `alSource` play/stop + offset |
| `SetVolume` | `rs2_audio_set_volume` | `AL_GAIN` from hundredths of a dB |
| `SetPos` | `rs2_audio_set_pos` | `AL_POSITION` |
| `SetListenerPos` / `Dir` / `Sens` | `rs2_audio_set_listener_*` | `alListener` + distance factor |

ctest: `rs2_audio_self_test` (`port/rs2_audio_test.cpp --self-test`). Do not link OpenAL in the `check` preset.
