# Charset seams (`_mbs*` / Imm*)

- **Issue**: [#62](https://github.com/lollipop-onl/railsim2-portable/issues/62) (parent [#9](https://github.com/lollipop-onl/railsim2-portable/issues/9); depends on [#46](https://github.com/lollipop-onl/railsim2-portable/issues/46))
- **ADR**: [charset-internal.md](charset-internal.md) (UTF-8 in-process; CP932 at file / Win32-compat boundaries)
- **Tree**: game `*.cpp` / `lib/*.cpp` at this document's commit. Counts exclude comments.
- **This slice does not implement conversion helpers or IME.** Later `#9` slices should replace **this closed set**, not walk the whole tree.

## Corrections to issue #62

| Claim in #62 | Fact |
|--------------|------|
| `_mbsinc` / `_mbslen` in `port/stub/mbstring.h` are byte-wise; `_mbsicmp` is `strcasecmp` | True. See [Stub status](#stub-status). |
| Call sites are scattered across `lib/editbox.cpp` (Imm*), `lib/texture.cpp`, `lib/mesh.cpp`, `CPlugin.cpp`, `CFileMode.cpp`, `CTreeElement.cpp` | True for **live** game / `lib/` callers. `lib/editbox.cpp` has **no** `_mbs*`; it is the Imm* island. |
| `_ismb*` must be inventoried with `_mbs*` | `_ismbblead` / `_ismbbtrail` are stub-only. **Zero** game or `lib/` callers. |

`_mbsicmp` is the **only** `_mbs*` / `_ismb*` token used outside `port/stub/`. Twelve call expressions in **five** files. Do not add `_mbsinc` / `_mbslen` / `_ismb*` replacements until a caller appears.

## Closed `_mbsicmp` set (compare)

All live uses are **case-insensitive compare**. None walk or measure CP932 characters.

| File | Function | Compared strings | Layer |
|------|----------|------------------|-------|
| `CPlugin.cpp` | `CPlugin::Compare` | `m_Name`, then `m_ID`, then `m_Author` | Name / author: plugin txt (I/O). ID: plugin folder name (filename). |
| `CPlugin.cpp` | `CPluginList::FindPlugin` | lookup `id` vs `ptr->m_ID`; then `id` vs `Default()` | Filename / `.rs2` plugin-id field. Miss -> `g_LackPlugin` path `"Type\\id"`. |
| `CFileMode.cpp` | `CFileListView::ConfirmRename` | list-column old basename vs `FixFileExt(..., "rs2")` new name | `.rs2` **filename**. Equality -> no-op (skip `rs2_rename`). |
| `CFileMode.cpp` | `CFileMode::ScanInputInterface` | chosen / current `m_NewFileName` vs `g_SaveFile->GetFileName()` | `.rs2` **filename**. Inequality -> Save As path; equality -> overwrite `Save`. |
| `CFileMode.cpp` | `CFileMode::ListFile` | each `CLayoutInfo::m_FileName` vs `g_SaveFile->GetFileName()` | `.rs2` **filename** from `rs2_list_dir`. Match highlights the open layout. |
| `CTreeElement.cpp` | `CTreeElement::Compare` | `m_String` vs `rhs->m_String` (dirs sort before files) | **UI label**. Plugin rows copy `CPlugin::m_Name` (`InsertItem`). Dir rows are in-memory folder labels (`lang(NewDir)` / rename). |
| `lib/texture.cpp` | `CTexList::Get` | cache `TEXINFO::strName` vs `_fullpath(strName)` (or resource name) | **Asset path** key. Not user-visible text. |
| `lib/mesh.cpp` | `CMeshList::Get` | cache `MESHINFO::strName` vs `_fullpath(strName)` | **Asset path** key. Then `MoveToFile` for sidecar textures. |

Do not expand a `_mbs*` rewrite into plugin `Load()` bodies, `CSaveFile` parsers, or `Language.txt`. Those files never call `_mbs*`.

`port/rs2_text` (#75) is the closed-set helper: `rs2_cp932_to_utf8` / `rs2_utf8_to_cp932` convert at file boundaries (iconv; `.rs2` on-disk bytes stay CP932), and `rs2_text_icmp` replaces the twelve `_mbsicmp` calls in the five files above by decoding CP932 to UTF-8 then ASCII-case-folding. IME (`Imm*`) is unchanged.

## File I/O vs internal UI

ADR rule: CP932 stays on disk; `std::string` after load becomes UTF-8. Classify the closed set that way so a helper slice knows **where** to convert.

### File / path boundary (keep on-disk bytes until a helper exists)

| Source | How it is filled | Reaches `_mbsicmp` as |
|--------|------------------|------------------------|
| `Layout/*.rs2` basename | `rs2_list_dir` / `CInputDialog` / `CListElement` rename | `CFileMode` three compares; `g_SaveFile->m_FileName` |
| Plugin directory name | `rs2_list_dir` -> `CPlugin` ctor `m_ID = id` | `Compare` / `FindPlugin` |
| `{Type}2.txt` / old `{Type}.txt` | `LoadBinaryText` + `AsgnString("PluginName"/"PluginAuthor")`; old form `fscanf` | `m_Name` / `m_Author` in `Compare` only |
| Mesh / texture file name | caller relative name + `_fullpath` / virtual cwd (`docs/porting/path-seams.md`) | `CTexList::Get` / `CMeshList::Get` |

`.rs2` **contents** that are user text (not compared with `_mbs*`):

- `CSaveFile::m_FileNote` <-> `AsgnString("Note")` / `fprintf Note` (`CSaveFile.cpp`). The file dialog copies this through `CEditCtrl` (`m_NoteEdit.GetRealtimeText()`), not `_mbsicmp`.
- Plugin ids **inside** the layout script are lookup keys for `FindPlugin` (boundary: script bytes today, UTF-8 after the ADR conversion).

`Language.txt` / `Config.txt` are I/O (`docs/porting/path-seams.md`) but have **zero** `_mbs*` callers. Out of this set.

### Internal UI (UTF-8 after the ADR; Imm* must deliver UTF-8)

| Store | Owner | How it is filled |
|-------|-------|------------------|
| `CEditBox::m_str` | committed buffer | `Create(pre)`, `AddChar` / `AddString`, clipboard `CF_TEXT`, `CompEnd` (IME result) |
| `CEditBox::m_comp` | composition preview | `GetCompStr` (`GCS_COMPSTR`); cleared after `CompEnd` |
| `CEditBox::m_show` | draw copy | `m_str` plus inserted `m_comp` while converting |
| `CEditCtrl::m_Text` | game edit field | `GetText` / `GetRealtimeText` |
| `CTreeElement::m_String` | plugin tree label | ctor / `EndRename` |
| `CListElement::m_String[0]` | list first column | ctor / `EndRename` |

`CPlugin::InsertItem` copies `m_Name` (file-origin) into `CTreeElement::m_String` (UI). After conversion, that copy is UTF-8; the on-disk `PluginName` line stays CP932.

Plugin-tree **directory** labels are UI-only (`CTreeDirElement::ConfirmRename` returns `true` and does not write a file). Plugin **file** rename goes `CTreeFileElement` -> `CPlugin::ConfirmRename`, which defaults to `false` (no disk write unless a subclass overrides). Layout `.rs2` rename **does** hit the filesystem (`CFileListView::ConfirmRename`).

## Closed Imm* set

`<imm.h>` is pulled once, from `lib/headers.h` (every udx TU). Game sources never include it directly.

### Calls that exist

| File | Function | API | Role |
|------|----------|-----|------|
| `lib/editbox.cpp` | `CEditBox::Create` | `ImmGetContext` | Store `m_hImc` from `svw.hWnd`. |
| `lib/editbox.cpp` | `Create` / `Release` | `ImmSetOpenStatus` | Force IME on/off from the `imm` argument (`-1` off, `>0` on, `0` leave). **Not declared** in `port/stub/imm.h`. |
| `lib/editbox.cpp` | `Release` | `ImmReleaseContext` | Drop `m_hImc`. |
| `lib/editbox.h` | `IsFEPOpen` | `ImmGetOpenStatus` | Gate `GetCompStr` and full-width space insert. Stub always `FALSE`. |
| `lib/editbox.h` | `GetFEPCursorPos` | `ImmGetCompositionString(..., GCS_CURSORPOS, NULL, 0)` | Composition caret. **Does not write** `m_str`. |
| `lib/editbox.cpp` | `ScanInput` | `GCS_COMPCLAUSE` size, then 1 byte of `GCS_COMPATTR` | Detect conversion (`attr != ATTR_INPUT`) vs raw IME input. |
| `lib/editbox.cpp` | `Render` | `GCS_COMPCLAUSE` + `GCS_COMPATTR` | Clause underlines. **Does not write** `m_str`. |
| `lib/editbox.cpp` | `GetCompStr` | `GCS_COMPSTR` | Preview bytes -> `m_comp` only. |
| `lib/editbox.cpp` | `GetResultStr` | `GCS_RESULTSTR` | Confirmed bytes -> `m_comp`, then `CompEnd` inserts into `m_str`. |
| `lib/window.cpp` | `WindowProc` `WM_IME_SETCONTEXT` | `rs2_ime_hide` (`#104`) | Was `ImmGetDefaultIMEWnd` + `SendMessage(..., WM_CLOSE)`. Stub hides no OS window; open / buffers stay. |

`ImmSetCompositionWindow` / `ImmSetCandidateWindow` are stub-only. No callers.

`lib/window.cpp` never feeds text. Hiding the default IME window is why `CEditBox` draws composition itself.

### How Imm* bytes enter `CEditBox` text

```
ImmGetContext -> m_hImc
        |
        +-- GCS_COMPSTR  -> GetCompStr  -> m_comp     (preview; GetText ignores)
        +-- GCS_RESULTSTR -> GetResultStr -> m_comp
                |
                CompEnd: m_str.insert(m_pos, m_comp); Clip(); m_comp.clear()
                |
                GetText(str) copies m_str
```

`GetText` is the **only** committed-text exit. Fan-out (do not rewrite these for IME; they already consume `std::string`):

| Consumer | Exit | Where the string goes |
|----------|------|------------------------|
| `CEditCtrl::FinishInput` / `Render` (focus loss) | `GetText` -> `m_Text` | Dialog / note / name fields. |
| `CEditCtrl::GetRealtimeText` | `GetText` while focused | `CFileMode` writes `g_SaveFile->m_FileNote` every scan (later `.rs2` `Note`). `CDiaDialog` writes `m_Name` / clock fields. |
| `CListElement::EndRename` | `GetText` -> `m_String[0]` | `CFileListView::ConfirmRename` may `rs2_rename` a `.rs2`. |
| `CTreeElement::EndRename` | `GetText` -> `m_String` | UI label; plugin file rename only if `ConfirmRename` succeeds. |

Non-IME paths into `m_str` (same buffer, not Imm*): `WM_CHAR` -> `DequeueChar` -> `AddChar` (ASCII `IsPrintChar`); `AddString("Å@")` when IME is open and Space is pressed; clipboard `CF_TEXT`. Caret move / delete uses `CharNext` / `CharPrev` (see [Adjacent, not this set](#adjacent-not-this-set)).

On the native `check` build, `ImmGetOpenStatus` is `FALSE` and `ImmGetCompositionString` returns `0`, so `m_comp` stays empty. Japanese composition is dead until an IME slice replaces this island.

`port/rs2_roundtrip_stubs.cpp` defines a no-op `CEditBox` for the roundtrip binary. That stub is **not** an Imm* caller and is out of the IME replacement set.

## Stub status

| Stub | Behavior | Effect on this set |
|------|----------|--------------------|
| `_mbsicmp` | `strcasecmp` | Wrong for CP932 (lead/trail bytes can look like ASCII letters). All 12 compares. |
| `_mbsinc` / `_mbslen` | `p+1` / `strlen` | Unused. |
| `_ismbblead` / `_ismbbtrail` | CP932 lead/trail tables | Unused. `Is2ByteChar` in `lib/editbox.h` duplicates the lead test and is also unused. |
| `ImmGetContext` | `nullptr` | `m_hImc` is null; further Imm* no-ops. |
| `ImmGetOpenStatus` | `FALSE` | `GetCompStr` skipped. |
| `ImmGetCompositionStringA` | `0` | No composition / result bytes. |
| `ImmSetOpenStatus` / `ImmGetDefaultIMEWnd` | **missing** | Live callers are gone (`#102` / `#104`). Header include in `lib/headers.h` remains. |

Do not grow these stubs in a helper slice except to compile. Behavior belongs in `lib/` / `port/` as [charset-internal.md](charset-internal.md) says.

## Adjacent, not this set

Later slices must **not** treat these as part of the `_mbs*` / Imm* closed set, but they share the same CP932 walk problem:

| API | Where | Notes |
|-----|-------|-------|
| `CharNext` / `CharPrev` | `CEditBox` backspace / delete / caret / `Clip` / `EliminateTabAndCRLF` | Stub in `port/stub/windows.h` is `+/- 1` byte. This is the **scan** seam for edit text. Parent `#9` / `#8` may replace it with UTF-8 walk after Imm* is gone. |
| `Is2ByteChar` | `lib/editbox.h` only | Dead. |
| `MultiByteToWideChar(CP_ACP, ...)` | `lib/music.cpp`, `lib/movie.cpp` | Those headers are commented out of `lib/udx.h`. Out of scope (`#16` / media). |
| `mbstowcs` | `lib/comm.cpp` | DirectPlay session name (`#11`). |
| GDI `HFONT` / `CStringTexture` | UI draw | `#16`. |

`#10` Save format and `#8` input implementation stay on their own inventories. This document only names where `_mbsicmp` and Imm* already are.

## What a follow-up slice may touch

One IME PR should replace `lib/editbox.cpp` + the `WM_IME_SETCONTEXT` hide in `lib/window.cpp` with backend `TEXTINPUT` (UTF-8) writing `m_str` / `m_comp`. Do not reimplement `CEditCtrl` / list / tree rename.

**Must not** (this inventory and those follow-ups):

- Implement `to_utf8` / `to_cp932` in *this* document's PR (already done as docs-only).
- Mass-convert game sources to UTF-8 (`encoding-guard.sh`).
- Rewrite `.rs2` / plugin on-disk format (`#10`).
- Touch `CMakeLists.txt`, `port/native_sources.txt`, `docs/porting/ffp-render-states.md`, or `docs/porting/input-seams.md`.

## Verification

Closed-set `_mbs*` / `_ismb*` / Imm* should stay inside the files named above (`port/stub/` excepted):

```bash
rg -n --glob '!build/**' --glob '!.git/**' --glob '!Distribution/**' --glob '!port/stub/**' \
  --glob '!docs/**' --glob '!port/rs2_roundtrip_stubs.cpp' \
  '\b_mbs|\b_ismb|ImmGet|ImmSet|ImmRelease|#include\s*<imm\.h>'
```

Expect: live Imm* calls gone from `lib/editbox.cpp` / `lib/editbox.h` / `lib/window.cpp`. Remaining: `lib/headers.h`'s `#include <imm.h>`. Live `_mbsicmp` is gone (stub + comments only).

`./scripts/check.sh` must stay green.

## Closed `_mbsicmp` replaced (`#87`)

- **Issue**: [#87](https://github.com/lollipop-onl/railsim2-portable/issues/87) (parent [#9](https://github.com/lollipop-onl/railsim2-portable/issues/9); depends on [#75](https://github.com/lollipop-onl/railsim2-portable/issues/75))
- **Entry**: `rs2_text_icmp` in `port/rs2_text.h` (added by #75)
- **Allowlist**: `CPlugin.cpp`, `CFileMode.cpp`, `CTreeElement.cpp` are in `port/native_sources.txt`

The twelve live `_mbsicmp` calls in the [closed compare set](#closed-_mbsicmp-set-compare) are gone. Those five files call `rs2_text_icmp` (CP932 decode, then ASCII A-Z fold). On-disk `.rs2` / plugin txt bytes stay CP932. IME (`Imm*`) is unchanged.

`lib/texture.cpp` and `lib/mesh.cpp` still use `rs2_text_icmp` but stay off the allowlist: they fail on D3D/GDI / `rmxfguid.h`, not on `_mbsicmp`. Do not grow those drawing stubs in a charset slice. `rs2_text_self_test` keeps the #75 icmp cases (ASCII fold, trail-byte trap, layout name).

## Port IME / TEXTINPUT stub backend (`port/rs2_ime`)

- **Issue**: [#100](https://github.com/lollipop-onl/railsim2-portable/issues/100) (parent [#9](https://github.com/lollipop-onl/railsim2-portable/issues/9); depends on [#75](https://github.com/lollipop-onl/railsim2-portable/issues/75) / [#87](https://github.com/lollipop-onl/railsim2-portable/issues/87))
- **Entry**: `rs2_ime_composition_utf8` / `rs2_ime_result_utf8` / `rs2_ime_get_composition_string_a` in `port/rs2_ime.h`
- **Backend hooks**: `rs2_ime_backend_*` (stub in `port/rs2_ime.cpp`; SDL2 later replaces composition + TEXTINPUT commit)

Check preset records UTF-8 composition (`SDL_TEXTEDITING`) and result (`SDL_TEXTINPUT`) only. There is no Imm COM, no SDL2, and no `lib/editbox.cpp` / `lib/window.cpp` allowlist. `rs2_ime_get_composition_string_a` is the `ImmGetCompositionStringA` stand-in: `RS2_IME_GCS_COMPSTR` (`0x0008`, Win32 `GCS_COMPSTR`) / `RS2_IME_GCS_RESULTSTR` (`0x0800`, Win32 `GCS_RESULTSTR`; not `0x1000` `GCS_RESULTCLAUSE`) return CP932 via `rs2_utf8_to_cp932`. In-process getters stay UTF-8 ([charset-internal.md](charset-internal.md)).

| Imm* / `CEditBox` | `port/rs2_ime` | Later SDL2 |
|-------------------|----------------|------------|
| `GCS_COMPSTR` -> `m_comp` | `rs2_ime_backend_set_composition` / `rs2_ime_composition_utf8` | `SDL_TEXTEDITING` |
| `GCS_RESULTSTR` -> `CompEnd` | `rs2_ime_backend_commit` / `rs2_ime_result_utf8` | `SDL_TEXTINPUT` |
| `ImmGetCompositionStringA` | `rs2_ime_get_composition_string_a` | same helper (CP932 at the Win32-compat edge) |
| cancel / focus loss | `rs2_ime_backend_clear` | stop text input + empty both |

`lib/editbox.cpp` now calls `rs2_ime_*` (#102). `lib/window.cpp` `WM_IME_SETCONTEXT` calls `rs2_ime_hide` (#104). Do not rewrite `CEditCtrl` / list / tree rename here. Do not add SDL2 to the `check` preset.

ctest: `rs2_ime_self_test` (`port/rs2_ime_test.cpp --self-test`) covers empty composition, ASCII commit, CP932 2-byte roundtrip, and clear.

## CEditBox Imm* routed through `port/rs2_ime` (`#102`)

- **Issue**: [#102](https://github.com/lollipop-onl/railsim2-portable/issues/102) (parent [#9](https://github.com/lollipop-onl/railsim2-portable/issues/9); depends on [#100](https://github.com/lollipop-onl/railsim2-portable/issues/100))
- **Entry**: `rs2_ime_composition_utf8` / `rs2_ime_result_utf8` / `rs2_ime_get_composition_string_a` / `rs2_ime_set_open` / `rs2_ime_is_open` in `port/rs2_ime.h`

`CEditBox` no longer calls Imm*. `Create` / `Release` use a dummy `m_hImc` session token and `rs2_ime_set_open` for the `imm` argument (`-1` off, `>0` on, `0` leave). `IsFEPOpen` is `rs2_ime_is_open`. `GetCompStr` / `GetResultStr` copy UTF-8 into `m_comp` (ADR in-process text); `CompEnd` still inserts `m_comp` into `m_str`. `ScanInput` / `Render` / `GetFEPCursorPos` call `rs2_ime_get_composition_string_a` with `RS2_IME_GCS_COMPCLAUSE` / `COMPATTR` / `CURSORPOS`, which return 0 in the record-only stub (no clause underlines). `lib/window.cpp` hide is `#104`.

`lib/editbox.cpp` stays off the allowlist: a check compile of that TU fails on leftover clipboard / common-dialog symbols and an include-order miss of `RS2_FLOAT_FMT` from `SystemCover.h`, not on Imm*. Follow-up (not this slice): `GMEM_DDESHARE` / `GMEM_MOVEABLE` / `lstrcpy` / `CF_TEXT` (`ClipCopy` / `ClipPaste`), `wsprintf` (`SelectFile`), and `#include "rs2_float.h"` before `SystemCover.h`. Do not add SDL2 to `check`. Do not rewrite `CEditCtrl` / list / tree rename. Game sources stay CP932.

ctest: `rs2_ime_self_test` also covers open-status on/off and zero-size COMPATTR / COMPCLAUSE / CURSORPOS.

## Window `WM_IME_SETCONTEXT` hide routed through `port/rs2_ime` (`#104`)

- **Issue**: [#104](https://github.com/lollipop-onl/railsim2-portable/issues/104) (parent [#9](https://github.com/lollipop-onl/railsim2-portable/issues/9); depends on [#102](https://github.com/lollipop-onl/railsim2-portable/issues/102))
- **Entry**: `rs2_ime_hide` in `port/rs2_ime.h`

`MessageProc` no longer calls Imm*. `WM_IME_SETCONTEXT` calls `rs2_ime_hide` (Win32: `ImmGetDefaultIMEWnd` + `SendMessage(WM_CLOSE)`). The stub is a no-op: there is no OS IME window, and hide must not flip `rs2_ime_set_open` or clear composition / result. `CEditBox` still draws composition itself.

`lib/window.cpp` stays off the allowlist. A check compile of that TU fails on leftover Win32 window / GDI symbols, not on Imm*. Follow-up (not this slice): `<windowsx.h>` (`GetWindowStyle` / `GetWindowExStyle`), `WNDCLASSEX` / `RegisterClassEx` / `CreateWindow`, `LoadIcon` / `LoadCursor` / `GetStockObject`, `AdjustWindowRectEx` / `SetWindowPos` / `GetDesktopWindow` / `GetWindowRect` / `GetMenu`, `BeginPaint` / `EndPaint` / `PAINTSTRUCT`, `SetWindowText` / `SendMessage`, `WM_IME_SETCONTEXT` / `WM_DISPLAYCHANGE` / `WM_SYSCOMMAND` / `SC_SCREENSAVE`, and A/W macros (`PeekMessage` / `DispatchMessage` / `DefWindowProc`). Do not grow those stubs in a charset slice. Do not add SDL2 to `check`. Do not allowlist `lib/editbox.cpp`. Game sources stay CP932.

ctest: `rs2_ime_self_test` also covers hide leaving open status and composition intact.
