# Sync dialog seams (`MessageBox` / `GetOpenFileName` / `GetSaveFileName`)

- **Issue**: [#69](https://github.com/lollipop-onl/railsim2-portable/issues/69) (parent [#12](https://github.com/lollipop-onl/railsim2-portable/issues/12))
- **Related**: [#64 input-seams](input-seams.md) (`MsgBox` / `MsgYesNo` boundary with `#8`)
- **Tree**: game `*.cpp` / `lib/*.cpp` at this document's commit. Counts exclude comments unless noted.
- **This slice does not implement SDL / nativefiledialog / in-game dialog backends.** Later `#12` slices should replace **this closed set**, not walk the whole tree.

## Corrections to issue #69 / parent #12

| Claim | Fact |
|-------|------|
| `MessageBox` ? 4 sites in `SystemCover.cpp`, 2 in `lib/input.cpp` | **5** direct `MessageBox` calls in `SystemCover.cpp` wrappers (`Dialog`, `ErrorDialog`, `YesNo`, `YesNoCancel`, `ShowLastError`) plus **2** in `lib/input.cpp`. **`lib/debug.h` `ASSERT` macro adds a 8th direct site** used from several `lib/` TUs via `FAILED_ASSERT`. |
| `GetOpenFileName` / `GetSaveFileName` in `SystemCover.cpp` and `lib/editbox.cpp` | True. Each TU wraps both APIs inside a local `SelectFile` helper. |
| Return-value branching (`return MessageBox(...)`) | True for `YesNo`, `YesNoCancel`, `MsgYesNo`. **`YesNo` / `YesNoCancel` / `MsgYesNo` have zero live callers** today; only the wrapper bodies branch on the Win32 return code. |
| `SelectFile` is part of the sync surface | Wrappers exist and are exported (`SystemCover.h`, `lib/editbox.h`), but **`SelectFile` has zero callers** in this tree. Path still lands in the caller-supplied `file` buffer when invoked. |
| `MessageBoxA` / `MessageBoxW` | **No game/`lib/` source uses `MessageBoxA` or `MessageBoxW` by name.** All live sites call `MessageBox` (Win32 macro Å® `MessageBoxA`; stub in `port/stub/windows.h` defines both). |

## Architecture

```
Win32 modal APIs                         Wrapper TUs              Live game callers
----------------                         ------------              -----------------
MessageBox                          -->  SystemCover.cpp           Dialog / ErrorDialog (many)
                                    -->  lib/input.cpp             MsgBox (2), MsgYesNo (0)
                                    -->  lib/debug.h ASSERT        FAILED_ASSERT in lib/* init

GetOpenFileName / GetSaveFileName   -->  SystemCover.cpp SelectFile (0 callers)
                                    -->  lib/editbox.cpp SelectFile (0 callers)

(already async, out of #12 sync set)
EnqueueCommonDialog(CWindowCtrl*)   -->  CSimpleDialog / CYesNoDialog / CInputDialog / ...
                                       ProcessCommonDialog() each frame
```

Game code almost never calls Win32 dialog APIs directly. It calls **`Dialog` / `ErrorDialog` / `MsgBox`** (and would call **`YesNo` / `YesNoCancel` / `SelectFile`** if linked). `#12` replaces those wrappers (and the two `lib/input.cpp` helpers documented under `#8`), not every `EnqueueCommonDialog` site.

## Closed sync set ? direct Win32 API sites

### `MessageBox` / `MessageBoxA` / `MessageBoxW`

| File | Lines | Wrapper / macro | Flags | Return use |
|------|-------|-----------------|-------|------------|
| `SystemCover.cpp` | 51?52 | `Dialog` | `MB_APPLMODAL` | void ? discarded |
| `SystemCover.cpp` | 69?70 | `ErrorDialog` | `MB_APPLMODAL` | n/a ? `ExitProcess(0)` after |
| `SystemCover.cpp` | 87?88 | `YesNo` | `MB_YESNO\|MB_APPLMODAL` | **`return` Win32 code** (`IDYES` / `IDNO`) |
| `SystemCover.cpp` | 103?104 | `YesNoCancel` | `MB_YESNOCANCEL\|MB_APPLMODAL` | **`return` Win32 code** (`IDYES` / `IDNO` / `IDCANCEL`) |
| `SystemCover.cpp` | 175 | `ShowLastError` | `MB_APPLMODAL` | void ? discarded |
| `lib/input.cpp` | 540 | `MsgBox` | `MB_OK` | void; then `FlushInputDevice()` |
| `lib/input.cpp` | 550?551 | `MsgYesNo` | `MB_YESNO\|MB_SYSTEMMODAL\|MB_ICONQUESTION` | **`return ret==IDYES`**; then `FlushInputDevice()` |
| `lib/debug.h` | 25 | `ASSERT(msg,b)` | `MB_SYSTEMMODAL` | **`return FALSE`** from enclosing function |

Stub only: `port/stub/windows.h` (`MessageBox` / `MessageBoxA` inline, always `0`).

### `GetOpenFileName` / `GetSaveFileName` / `OPENFILENAME`

| File | Function | Lines | API | Path output | Return use |
|------|----------|-------|-----|-------------|------------|
| `SystemCover.cpp` | `SelectFile(HWND,Åc)` | 120?137 | `OPENFILENAME`; `GetSaveFileName` if `flag`, else `GetOpenFileName` | `ofn.lpstrFile` Å® caller `file[]` | **`return ret`** (`BOOL`) |
| `lib/editbox.cpp` | `SelectFile(char*,Åc)` | 512?538 | same pattern; `ofn.hwndOwner = svw.hWnd`; filter built from `def`/`ext` | same | **`return ret`** (`BOOL`) |

**Zero live callers** for either `SelectFile` overload. When used, selected path is written into the caller buffer; cancel leaves `file` zeroed (`memset` before dialog).

## Wrapper inventory and live callers

### `#8` defer ? `lib/input.cpp` (do not mix with `#12`)

Documented in [input-seams.md](input-seams.md). Replace signatures in `#12` only after `#8` input backend lands; do not stub here during `#8` work.

| Wrapper | Callers | Classification |
|---------|---------|----------------|
| `MsgBox` | `lib/main.cpp` 78, 85 (screensaver guard / single-instance) | **#8 pass-through** (also **M4** startup guard) |
| `MsgYesNo` | none | **#8 pass-through** (exported in `lib/input.h`, dead today) |

Both call `FlushInputDevice()` after the modal closes.

### `SystemCover.cpp` wrappers

| Wrapper | Behavior after modal | Live callers | Classification |
|---------|---------------------|--------------|----------------|
| `Dialog` | returns to caller | `Capture.cpp` 267, 272, 426, 431, 436, 470, 476 (AVI / video capture failures) | **error notify** (OK only; no return branch). Optional feature, not M4 core. |
| `ErrorDialog` | `ShowCursor(TRUE)`, `DestroyWindow(svw.hWnd)`, `ExitProcess(0)` | see table below | **M4** fatal paths + **editor** internal errors |
| `YesNo` | returns `IDYES` / `IDNO` | none | **dead API** (return-branching wrapper) |
| `YesNoCancel` | returns tri-state Win32 code | none | **dead API** |
| `ShowLastError` | `FormatMessage` + OK box | none | **dead API** |
| `SelectFile` | returns `BOOL`, path in buffer | none | **dead API** (file picker seam for follow-on `#12` / `#70`) |

#### `ErrorDialog` live callers (fatal ? all classifications **M4** unless noted)

| File | Lines | Context |
|------|-------|---------|
| `RailSim2.cpp` | 224, 232, 239, 292, 300, 307 | Plugin / env / skin / surface init failure at startup |
| `Script.cpp` | 39, 61, 67 | Script parse / compile failure |
| `Network.cpp` | 459, 682, 689 | Layout hash mismatch; transfer protocol errors |
| `CSaveFile.cpp` | 856, 858 | Missing plugin list on load |
| `lib/object.cpp` | 672 | Unexpected index buffer format |
| `CTrainGroup.cpp` | 45, 47, 62, 64, 1147 | Net sync errors; consist buffer sanity |
| `CTrainEditMode.cpp` | 435, 449 | Invalid move index | **editor** |
| `CSceneEditMode.cpp` | 250 | Invalid move index | **editor** |
| `CStationPlugin.cpp` | 223, 310, 325 | Platform / station plugin load | **editor** |
| `CStation.cpp` | 309 | Platform parent parts missing | **editor** |
| `CRailConnector.cpp` | 16 | Static point option id | **editor** |
| `CModelInst.cpp` | 22 | Static switch option id | **editor** |
| `CJobTimer.cpp` | 37, 46, 54 | Timer misuse (debug guard) | **error notify** / dev guard |

Roundtrip stub: `port/rs2_roundtrip_stubs.cpp` `ErrorDialog` (no Win32); not a live game path.

### `ASSERT` / `FAILED_ASSERT` ? direct `MessageBox` via macro

Expands to sync `MessageBox` + `return FALSE`. Used during **`lib/` initialization**, not editor modes.

| File | Representative lines | Context |
|------|---------------------|---------|
| `lib/graphic.cpp` | 39 (`ASSERT`), 113 (`FAILED_ASSERT`) | D3D create / device create |
| `lib/input.cpp` | 26, 68, 73, 78, 103, 107, 111 | DirectInput device setup |
| `lib/sound.cpp` | 23, 78, 84, 92 | DirectSound buffer / listener |
| `lib/comm.cpp` | 29, 47, 78, 106, 256, 300, 370 | DirectPlay session setup |
| `lib/music.cpp` | 16, 98, 110 | DirectMusic (module unused from game today) |
| `lib/movie.cpp` | 17, 24, 30 | DirectShow (compiled out via `NO_MOVIE`) |

Classification: **M4** for `graphic` / `input` / `sound` / `comm` init failures; **out of active set** for `music` / `movie` until those modules ship.

## Classification summary

| Tag | Meaning | Members |
|-----|---------|---------|
| **M4** | Needed before M4 bring-up / runtime integrity | `ErrorDialog` startup & network paths; `MsgBox` single-instance; init `ASSERT`/`FAILED_ASSERT` in active `lib/` |
| **error notify** | OK-only; caller continues | `Dialog` in `Capture.cpp`; `CJobTimer` misuse messages |
| **editor** | Build / layout editing only | `ErrorDialog` in edit modes, station/rail/switch helpers |
| **#8 pass-through** | Owned by input slice, not `#12` body | `MsgBox`, `MsgYesNo` in `lib/input.cpp` |
| **dead API** | Wrapper + Win32 site exists; zero callers | `YesNo`, `YesNoCancel`, `ShowLastError`, both `SelectFile` overloads, `MsgYesNo` |

Return-value branching that **`#12` must preserve when rewiring callers** (only relevant if a dead API gains callers or existing wrapper is invoked):

| Wrapper | Branch semantics |
|---------|------------------|
| `YesNo` | Win32 `IDYES` / `IDNO` |
| `YesNoCancel` | Win32 `IDYES` / `IDNO` / `IDCANCEL` |
| `MsgYesNo` | C `bool`: `IDYES` vs anything else |
| `SelectFile` | `TRUE` = path in buffer; `FALSE` = cancel / failure |
| `ErrorDialog` | does not return |
| `Dialog` / `MsgBox` | void |

## Delta vs `EnqueueCommonDialog`

Most in-game user messaging **already** uses the async queue:

| Aspect | Sync set (`#12`) | Async `EnqueueCommonDialog` |
|--------|------------------|----------------------------|
| Blocking | Win32 modal loop; halts caller until dismissed | Queue `CWindowCtrl*`; `ProcessCommonDialog()` advances when `GetWindowState()` |
| HWND | `GetActiveWindow()` / `svw.hWnd` / `NULL` | In-game skin widgets (`CPushButton`, `CStaticCtrl`, Åc) |
| OK errors | `Dialog`, `MsgBox` | `EnqueueCommonDialog(new CSimpleDialog(text, title))` ? **40+ live sites** (modes, network, save, plugins) |
| Yes / No | `YesNo`, `MsgYesNo` (unused) | `CYesNoDialog` + optional `CMenuCommand` callbacks (`CTrainGroupTemplate`, `CStruct`, `CStation`, `CFileMode`, Åc) |
| Text input | ? | `CInputDialog` / `CMultiInputDialog` |
| File pick | `SelectFile` Å® `GetOpen*FileName` (unused) | **No equivalent** in tree; needs new async file seam (later `#12` / `#70`) |
| Fatal exit | `ErrorDialog` Å® destroy window + `ExitProcess` | No async fatal path; `#12` needs explicit shutdown contract |
| Input flush | `MsgBox` / `MsgYesNo` call `FlushInputDevice()` | Dialog widgets read `GetKey` in `ScanInputWindow()`; no flush helper |
| Web (#14) | Cannot port sync Win32 modals | Queue pattern maps to frame-driven UI |

**Replacement sketch (implementation out of scope for #69):**

- `Dialog` / non-fatal errors Å® `CSimpleDialog` via `EnqueueCommonDialog` (same as rest of game).
- `YesNo` / `YesNoCancel` / `MsgYesNo` Å® `CYesNoDialog` (+ commands or polled `CheckYes`/`CheckNo`).
- `SelectFile` Å® new queued file-request type backed by native/web picker; callback with path instead of sync `BOOL`.
- `ErrorDialog` Å® fatal error service (log + controlled teardown), not a modal queue item.
- `MsgBox` / `MsgYesNo` Å® keep exported through `#8`; re-home to async dialog + `FlushInputDevice` when `#12` touches input layer.

## Out of scope (this slice / `#69`)

- SDL / nativefiledialog / in-game dialog implementation
- Rewriting `lib/input.cpp` modal helpers (coordinate with `#8` / `#64`)
- `EnqueueCommonDialog` caller migrations (already async)
- Rendering ([#5](https://github.com/lollipop-onl/railsim2-portable/issues/5)), audio ([#7](https://github.com/lollipop-onl/railsim2-portable/issues/7)), GDI ([#16](https://github.com/lollipop-onl/railsim2-portable/issues/16))
- Follow-on file-dialog backend slice ([#70](https://github.com/lollipop-onl/railsim2-portable/issues/70) and later `#12` work)

## `#12` contract (summary)

Replace **this closed sync set** ? eight `MessageBox` sites, four `Get*FileName` sites, and their wrappers ? while:

1. Leaving **`EnqueueCommonDialog` call sites** unchanged (already non-blocking).
2. Not absorbing **`lib/input.cpp` `MsgBox` / `MsgYesNo`** into `#12` without `#8` coordination ([input-seams.md](input-seams.md)).
3. Preserving **return semantics** for `SelectFile` / yes-no wrappers when callers appear.
4. Redesigning **`ErrorDialog`** as fatal shutdown, not an in-frame dialog.

After `#12` planning, a reader should be able to answer: Ågswap these wrappers and Win32 calls, and every remaining sync modal is accounted for.Åh

## Dialog / ErrorDialog replacement (#97)

- **Issue**: [#97](https://github.com/lollipop-onl/railsim2-portable/issues/97) (parent [#12](https://github.com/lollipop-onl/railsim2-portable/issues/12))
- **Entry**: `SystemCover.cpp` `Dialog` / `ErrorDialog` (void signatures unchanged)
- **Allowlist**: `SystemCover.cpp` in `port/native_sources.txt`

`Dialog` still formats into `g_FlashBuf`, then `EnqueueCommonDialog(new CSimpleDialog(text, DIALOG_TITLE))`. `CInterface::Init` copies the label into `std::string`, so the flash buffer may be reused after return. This is the same OK-only queue as the rest of the game; the caller does not block.

`ErrorDialog` is not queued. It logs to stderr, then the existing teardown (`ShowCursor(TRUE)`, `DestroyWindow(svw.hWnd)`, `ExitProcess(0)`). `ExitProcess` is the port exit in `port/stub/windows.h` (no-op under the check firewall). No sync `MessageBox`.

`RS2_ROUNDTRIP` keeps `port/rs2_roundtrip_stubs.cpp` `ErrorDialog` as log-only so `Sample.rs2` load can fail into `CSynErr` without process teardown.

Do not change `YesNo` / `YesNoCancel` / `SelectFile` / `ShowLastError` / `ColorDialog`, or `lib/input.cpp` `MsgBox` / `MsgYesNo`. Those wrappers still compile through the firewall (`OPENFILENAME` / `CHOOSECOLOR` / `FormatMessage` stubs in `port/stub/windows.h` are compile-only; not a file-picker backend).
