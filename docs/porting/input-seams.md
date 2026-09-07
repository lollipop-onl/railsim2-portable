# Input seams (DirectInput8 / `DIK_*` / `ScanInputDevice`)

- **Issue**: [#64](https://github.com/lollipop-onl/railsim2-portable/issues/64) (parent [#8](https://github.com/lollipop-onl/railsim2-portable/issues/8))
- **Tree**: game `*.cpp` / `lib/*.cpp` at this document's commit. Counts exclude comments unless noted.
- **This slice does not implement an input backend or key-config code.** Later `#8` slices should replace **this closed set**, not walk the whole tree.

## Corrections to issue #64

| Claim in #64 | Fact |
|--------------|------|
| `lib/input.cpp` calls `DirectInput8Create` | True. Only TU that touches DirectInput COM APIs. |
| `CApp::Run()` calls `ScanInputDevice()` every frame | True (`lib/main.cpp`). `RailSim2.cpp` `Opening()` and `CCursor::ScanInput()` also call it; `CGameMode.cpp` has a commented duplicate. |
| `port/stub/dinput.h` defines some `DIK_*` only | True (44 scan codes). Six **`DIK_*` tokens used by game/`lib/` are missing**; four more menu hotkeys (`DIK_5`?`DIK_8`, `DIK_F3`/`F5`/`F6`) are referenced only via `DIK_1+n` / `DIK_F1+i` expressions. |
| Camera inputs are M3-critical | True for **`CCamera`** (game) and **`lib/view_ctrl.cpp`** helpers. `KeyLook` / `MouseLook` have **zero callers** today but use the same seam. |
| `lib/input.cpp` has two sync `MessageBox` sites | True (`MsgBox`, `MsgYesNo`). Pass to [#12](https://github.com/lollipop-onl/railsim2-portable/issues/12); do not cut in `#8`. |

## Architecture

```
DirectInput8 / Win32 cursor APIs          lib/input.cpp (poll thread + Scan*)
        |                                          |
        v                                          v
   svi.key/btn/wheel/joy buffers  ----->  GetKey / GetButton / GetWheel / GetCursor* / GetJoy
                                                    |
                    game + lib callers (CCamera, UI widgets, modes, editbox, …)
```

Game code never includes `<dinput.h>` directly; `lib/headers.h` pulls it in for `lib/` TUs. The **public seam** for `#8` is the functions declared in `lib/input.h`, not raw DirectInput types.

Polling model (keep structure):

1. Background thread (`InputPollingThread`) calls `InputPollOnce()` → `GetDeviceState` on keyboard/mouse.
2. Main thread `ScanInputDevice()` merges poll buffers, then `ScanKeyboard` / `ScanMouse` / `ScanJoyStick`.
3. `GetKey` / `GetButton` / `GetJoy` expose edge/hold semantics (`S_FREE` … `S_HOLD`).

Cursor position is **not** taken from `DIMOUSESTATE.lX/lY`. `ScanMouse` reads Win32 `GetCursorPos` (see comment in `lib/input.cpp`). Relative drag uses `CCursor::m_Delta` (center-relative offset), not DI relative mode.

## Closed DirectInput / poll set

All symbols below live in **`lib/input.cpp`** unless noted.

| API / symbol | Function | Role |
|--------------|----------|------|
| `DirectInput8Create` | `InitDirectInput` | Create `svi.pDI` |
| `IDirectInput8::CreateDevice` | `InitKeyboard`, `InitMouse`, `EnumJoyCallback` | `GUID_SysKeyboard`, `GUID_SysMouse`, joy instances |
| `IDirectInput8::EnumDevices` | `InitJoyStick` | `DI8DEVCLASS_GAMECTRL`, `DIEDFL_ATTACHEDONLY` |
| `IDirectInputDevice8::SetDataFormat` | init paths | `c_dfDIKeyboard`, `c_dfDIMouse`, `c_dfDIJoystick2` |
| `IDirectInputDevice8::SetCooperativeLevel` | init paths | `DISCL_BACKGROUND \| DISCL_NONEXCLUSIVE` |
| `IDirectInputDevice8::Acquire` / `Unacquire` | init/free + `DIERR_INPUTLOST` recovery | keyboard, mouse, joy |
| `IDirectInputDevice8::GetDeviceState` | `InputPollOnce`, `ScanJoyStick` | keyboard 256 B, `DIMOUSESTATE`, `DIJOYSTATE2` |
| `IDirectInputDevice8::Poll` | `ScanJoyStick` | before joy `GetDeviceState` |
| `IDirectInputDevice8::EnumObjects` | `InitJoyStick` | `DIDFT_AXIS` → `EnumAxisCallback` |
| `IDirectInputDevice8::SetProperty` | `EnumAxisCallback` | `DIPROP_RANGE` on joy axes |
| `ScanInputDevice` | see [callers](#scaninputdevice-callers) | frame/input entry |
| `InputPollOnce` | poll thread + `ScanInputDevice` when `g_InputPollCount==0` | merges DI into `*Poll` buffers |
| `FlushInputDevice` | `MsgBox` / `MsgYesNo`, startup | clears `svi` buffers |

### `ScanInputDevice` callers

| File | Context |
|------|---------|
| `lib/main.cpp` | `CApp::Run()` ? **every frame** |
| `RailSim2.cpp` | `Opening()` message loop |
| `CCursor.cpp` | `CCursor::ScanInput` (also calls `FixCursor` → `SetCursor` center) |

### Win32 cursor / clip (paired with input, not DirectInput)

| API | File | Role |
|-----|------|------|
| `GetCursorPos` / `ScreenToClient` | `lib/input.cpp` `GetCursorPosClient` | absolute client cursor for `svi.cur` |
| `ClientToScreen` / `SetCursorPos` | `lib/input.cpp` `SetCursor` | warp cursor (fullscreen centering, `MouseLook`, `CCursor`) |
| `ShowCursor` | `lib/input.cpp` `InitMouse`, `CCursor`, `lib/window.cpp`, `SystemCover.cpp` | hide OS cursor in fullscreen / drag |
| `ClipCursor` | `CCursor.cpp`, `lib/window.cpp`, `RailSim2.cpp`, `CGameMode.cpp` | confine cursor to client rect |

### Mouse wheel

| Layer | File | Mechanism |
|-------|------|-----------|
| Poll | `lib/input.cpp` `InputPollOnce` | `DIMOUSESTATE.lZ` → `svi.wheelPoll` |
| Frame | `lib/input.cpp` `ScanMouse` | `svi.wheel` ← accumulated poll |
| Read | `GetWheel()` | callers below |

`GetWheel()` consumers: `CCamera::ScanInput` (zoom), `CSceneryMode` (map zoom + `DIK_PRIOR`/`NEXT` fallback), `CListView`, `CMultiStatic`, `CPluginTree` (list scroll). All treat sign as scroll direction; magnitude can accumulate across frames in camera/map paths.

### Relative movement (`CCursor` → camera / scenery)

| Step | File | Behavior |
|------|------|----------|
| `CCursor::ScanInput` | `CCursor.cpp` | `ScanInputDevice`; `m_Delta = GetCursorXY() - screen center`; updates virtual `m_Pos` when unlocked |
| `CCursor::FixCursor` / `Lock` | `CCursor.cpp` | warp hardware cursor to center (`SetCursor`) |
| `CCamera::ScanInput` | `CCamera.cpp` | `g_Cursor.GetDelta()` for RMB orbit (`CAM_HEAD`/`CAM_PITCH`) and L/MMB slide (`Slide`) |
| `CCamera::Slide` | `CCamera.cpp` | pan/slide from delta × distance × mode |
| `CSceneryMode` | `CSceneryMode.cpp` | map pan from `g_Cursor.GetDelta()` when arrow mode |
| `CTrainGroup` | `CTrainGroup.cpp` | train-cursor drag via `GetDelta()` |

No caller reads mouse relative axes from DirectInput.

## Public read API (backend must preserve)

Declared in `lib/input.h`. Game + `lib/` depend on these names and semantics.

| Function | DirectInput / Win32 source | Notes |
|----------|---------------------------|-------|
| `GetKey(int id)` | `svi.key[]` indexed by **`DIK_*` scan code** | Returns `S_FREE`…`S_HOLD` |
| `CheckKeyDown()` | scans all 256 keys | first `S_PUSH`, else first held |
| `GetButton(int id)` | `svi.btn[]`, **`DIM_*` index 0?2** | not Windows VK |
| `GetWheel()` | `svi.wheel` | `LONG`, frame delta |
| `GetCursorXY` / `X` / `Y` | `svi.cur` | client pixels |
| `IsCursorInside()` | `svi.inside` | windowed hit test |
| `SetCursor(x,y)` | Win32 warp | used by mouse-look + cursor lock |
| `GetJoy(n, id)` | `svi.joy[][]`, **`DIJ_*` index** | axis thresholds ±500 in `ScanJoyStick` |
| `EnableJoyStick` | `svi.fJoy` flag | only `lib/view_ctrl.cpp` debug display uses `GetJoy` today |
| `DequeueChar` / `OnChar` | WM_CHAR queue | editbox text; orthogonal to DIK polling |
| `FlushKey` / `FlushInputDevice` | buffer clears | after modal UI |

Stubs for roundtrip tests: `port/rs2_roundtrip_stubs.cpp` (`GetKey`, `GetButton`), `port/rs2_roundtrip_seams.cpp` (`GetWheel`).

## `DIK_*` / `DIM_*` / `DIJ_*` vs `port/stub/dinput.h`

### `DIM_*` and `DIJ_*`

Defined in **`lib/input.h`** (not in `port/stub/dinput.h`). Indices are game-local:

| Family | Constants | Used by |
|--------|-----------|---------|
| `DIM_*` | `LEFT=0`, `RIGHT=1`, `MIDDLE=2` | UI widgets, `CCamera`, `CSceneryMode`, … |
| `DIJ_*` | `UP`…`BOTTOM` (0?5), `BT1`…`BT8` (6?13) | `ScanJoyStick` mapping; debug `ShowJoyStickInfo` only |

### `DIK_*` ? stub vs game/`lib/`

Scan-code values follow DirectInput `DIK_*` numbering. **`port/stub/dinput.h` must grow** before `lib/input.cpp` enters `port/native_sources.txt`.

| Token | In stub? | Referenced from (representative) |
|-------|----------|----------------------------------|
| `DIK_ESCAPE` | yes | dialogs, build modes, `lib/editbox.cpp` |
| `DIK_0` … `DIK_4` | yes | `CSceneryMode` scene slots (+ numpad twins) |
| **`DIK_5` … `DIK_8`** | **no** | `CGameMode` menu icons (`DIK_1+j-1`, `MODE_SUB` up to 8) |
| `DIK_BACK`, `DIK_DELETE` | yes | lists, rails, structs, plugin tree |
| `DIK_TAB` | yes | focus cycle (`CInterface`, train modes) |
| `DIK_RETURN`, `DIK_NUMPADENTER` | yes | dialogs, buttons, editbox |
| `DIK_LCONTROL`, `DIK_RCONTROL` | yes | `SystemCover.h` `CheckCtrl()` |
| `DIK_LSHIFT`, `DIK_RSHIFT` | yes | `CheckShift()` |
| `DIK_LALT`, `DIK_RALT` | yes | `CheckAlt()` |
| `DIK_SPACE` | yes | buttons, rail build, train view |
| `DIK_A` | **no** | `lib/view_ctrl.cpp` WASD move |
| `DIK_S`, `DIK_D` | yes | WASD + scenery shortcuts |
| **`DIK_W`** | **no** | `lib/view_ctrl.cpp` WASD |
| **`DIK_X`, `DIK_C`, `DIK_V`, `DIK_B`** | **no** | editbox clipboard macros; `OBJ_CTRL` macro in `lib/object.h` (`V`/`B` rotate) |
| `DIK_Y`, `DIK_N`, `DIK_M`, `DIK_Z` | yes | dialogs, scenery undo/redo / map |
| `DIK_SLASH` | yes | scenery map toggle |
| `DIK_F` | yes | `Network.cpp` Ctrl+F |
| `DIK_F1`, `DIK_F2` | yes | menu icons (`DIK_F1+i`) |
| **`DIK_F3`, `DIK_F5`, `DIK_F6`** | **no** | menu icons (`DIK_F1+i`, `i=0..5`) |
| `DIK_F4` | yes | Alt+F4 exit (`RailSim2`, `CGameMode`) |
| `DIK_F11`, `DIK_F12` | yes | scenery wireframe / `Capture.cpp` screenshot |
| `DIK_HOME`, `DIK_END` | yes | camera local focus, train group, editbox |
| `DIK_UP`, `DIK_DOWN`, `DIK_LEFT`, `DIK_RIGHT` | yes | camera, lists, plugins, rails, `OBJ_CTRL` |
| `DIK_PRIOR`, `DIK_NEXT` | yes | camera zoom, scenery map zoom |
| `DIK_NUMPAD0` … `DIK_NUMPAD4` | yes | scenery scene shortcuts |

`CToggleIcon` stores hotkeys as `DWORD m_HotKey` (`GetKey(m_HotKey)`). Values are **`DIK_*` constants**, not configurable through `Config.txt` in this tree.

### DirectInput types / APIs used but not in stub

When `lib/input.cpp` is allowlisted, extend `port/stub/dinput.h` (or a sibling stub) for at least:

- `DirectInput8Create`, `IID_IDirectInput8`
- `GUID_SysKeyboard`, `GUID_SysMouse`
- `c_dfDIKeyboard`, `c_dfDIMouse`, `c_dfDIJoystick2`
- `DISCL_*`, `DI8DEVCLASS_GAMECTRL`, `DIEDFL_*`, `DIERR_INPUTLOST`, `DI_OK`, `DIENUM_*`
- `DIMOUSESTATE`, `DIJOYSTATE2`
- `DIPROPRANGE`, `DIPROPHEADER`, `DIPH_BYID`, `DIPROP_RANGE`, `DIDFT_AXIS`
- `IDirectInput8::EnumDevices`
- `IDirectInputDevice8::EnumObjects`, `SetProperty`, `Poll`

## M3-required camera inputs

Mark **`M3`** = needed for 3D bring-up / viewport navigation (parent `#8` backend must satisfy these before M3 modes are usable).

### `lib/view_ctrl.cpp` (udx helpers)

| Function | Inputs | M3 |
|----------|--------|-----|
| `KeyLook` | `DIK_UP/DOWN/LEFT/RIGHT`, `DIK_W/A/S/D` | yes (no callers today; same seam as live camera) |
| `MouseLook` | above + `GetCursorXY`, `SetCursor` warp | yes |
| `ShowKeyboardInfo` | arrow + `DIK_Z/X/C` | debug overlay only |
| `ShowMouseInfo` | cursor, `DIM_*`, `GetWheel` | debug overlay only |
| `ShowJoyStickInfo` | `DIJ_*` | debug overlay only |

### `CCamera` (`CCamera.cpp`)

| Function | Inputs | M3 |
|----------|--------|-----|
| `ControlLocal` | `DIK_HOME`, `DIK_END` (+ Ctrl/Shift modifiers) | yes |
| `ScanInput` | `GetWheel`, `DIK_PRIOR`/`NEXT`, `DIM_LEFT`/`MIDDLE`/`RIGHT`, `g_Cursor.GetDelta()` | yes |
| `Slide` | `g_Cursor.GetDelta()` | yes (called from `ScanInput`) |

`GetCamera()->ScanInput(...)` is invoked from neutral/scenery/rail/struct/train/3D plugin modes (see grep targets in `CCamera::ScanInput` call sites). **`CCursor::ScanInput`** must run in the same frames (provided by `CSceneryMode`, `CInterfaceMode`, etc.).

Non-M3 but same API: widget `ScanInput` methods (`CPushButton`, `CListView`, …) ? needed for editor UI, not for bare 3D viewport smoke.

## `MessageBox` ? pass to #12

| Function | File | Lines | Behavior |
|----------|------|-------|----------|
| `MsgBox` | `lib/input.cpp` | sync `MessageBox` + `FlushInputDevice` | also called from `lib/main.cpp` (preview / single-instance errors) |
| `MsgYesNo` | `lib/input.cpp` | sync `MessageBox` YES/NO + flush | **no game callers** yet; exported in `lib/input.h` |

Replace with non-blocking / portable dialog in [#12](https://github.com/lollipop-onl/railsim2-portable/issues/12). Do not stub out in `#8` input work.

## Out of scope (this slice / `#64`)

- SDL or other input backend implementation
- Key-config / `Config.txt` schema changes ([#10](https://github.com/lollipop-onl/railsim2-portable/issues/10) compat is separate)
- IME ([#9](https://github.com/lollipop-onl/railsim2-portable/issues/9))
- Modal dialog replacement ([#12](https://github.com/lollipop-onl/railsim2-portable/issues/12))
- Rendering ([#5](https://github.com/lollipop-onl/railsim2-portable/issues/5))
- Adding `lib/input.cpp` to `port/native_sources.txt` (follow-on allowlist slice)

## `#8` backend contract (summary)

Replace **`lib/input.cpp`** implementation (and grow **`port/stub/dinput.h`** as needed) while keeping:

1. **`ScanInputDevice()`** call pattern in `CApp::Run()` unchanged.
2. **`lib/input.h` public API** and `S_*` / `DIM_*` / `DIJ_*` semantics unchanged.
3. **`DIK_*` scan codes** as the key identity seen by game code (including the ten stub-missing tokens above).
4. **Win32 cursor warp + center-relative delta** behavior used by `CCursor` / `CCamera` / `MouseLook`.
5. **`MsgBox` / `MsgYesNo`** signatures until `#12`.

After `#8`, a reader should be able to answer: swap these functions and stub symbols, and M3 camera + editor input still compile and behave.

## Port input backend (`port/rs2_input`)

- **Issue**: [#82](https://github.com/lollipop-onl/railsim2-portable/issues/82) (parent [#8](https://github.com/lollipop-onl/railsim2-portable/issues/8))
- **Entry**: `rs2_input_poll_once` / `rs2_input_scan_*` / `rs2_input_edge` in `port/rs2_input.h`
- **Backend hooks**: `rs2_input_backend_*` (stub in `port/rs2_input.cpp`; SDL2 poll in `port/rs2_input_sdl.cpp` when `RS2_HAVE_SDL2`, see below)

`lib/input.cpp` no longer calls DirectInput COM. `ScanInputDevice` still merges poll buffers, then `ScanKeyboard` / `ScanMouse` / `ScanJoyStick`. Cursor position is client coordinates from the backend (Win32 `GetCursorPos` + `ScreenToClient` later; injected in the stub). `GetKey` / `GetButton` / `GetJoy` keep `S_FREE`..`S_HOLD` via `rs2_input_edge`. `port/stub/dinput.h` now defines the inventory-missing `DIK_*` tokens (`DIK_5`..`DIK_8`, `DIK_A`/`W`/`X`/`C`/`V`/`B`, `DIK_F3`/`F5`/`F6`).

The check preset links the stub only and does **not** start the original `CCrtThread` poll loop (`_beginthreadex` in `port/stub/process.h` would run it inline). `ScanInputDevice` already calls `InputPollOnce` when `g_InputPollCount==0`. Do not add SDL2 to the `check` preset. `MsgBox` / `MsgYesNo` stay sync `MessageBox` until [#12](https://github.com/lollipop-onl/railsim2-portable/issues/12).

`rs2_input_self_test` covers `GetKey` edge/hold, `GetWheel` accumulation, client cursor + inside, joystick axis thresholds, and the stub-missing `DIK_*` constants.

## SDL2 poll backend (`port/rs2_input_sdl.cpp`)

- **Issue**: [#122](https://github.com/lollipop-onl/railsim2-portable/issues/122) (parent [#8](https://github.com/lollipop-onl/railsim2-portable/issues/8))
- **On (`RS2_HAVE_SDL2`)**: `rs2_input_backend_poll_keys` / `_mouse` / `_get_cursor` / `_set_cursor` read SDL. Scancodes map through a closed table onto the `DIK_*` already in `port/stub/dinput.h` (M3 camera: arrows, WASD, HOME/END, PRIOR/NEXT, plus the rest of that stub set). Unknown scancodes stay 0. Mouse `DIM_LEFT` / `RIGHT` / `MIDDLE` come from `SDL_BUTTON_*`. Wheel is `SDL_MOUSEWHEEL.y * 120` (Win32 `WHEEL_DELTA`).
- **Cursor**: if an FFP window exists, client coordinates come from that native `SDL_Window` (`SDL_GetMouseState` when it has mouse focus, else global minus window origin). `set_cursor` is `SDL_WarpMouseInWindow`. No FFP window: poll / get / set do not crash; cursor is the last `set_cursor` value. This TU never creates a window.
- **Off (check/CI)**: existing stub in `port/rs2_input.cpp`. `rs2_input_self_test` stays on that path and does not link SDL2. Joystick stays stub. Do not add SDL2 to the `check` preset. `MsgBox` / IME / OpenAL are other issues.
