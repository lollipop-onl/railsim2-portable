# GDI seams (`CPixelbit` / `CStringTexture` / `HFONT` / blit / clipboard)

- **Issue**: [#70](https://github.com/lollipop-onl/railsim2-portable/issues/70) (parent [#16](https://github.com/lollipop-onl/railsim2-portable/issues/16))
- **Related**: [#17](https://github.com/lollipop-onl/railsim2-portable/issues/17) (`Capture.cpp` `BitBlt` no-op / capture backend), [#9](https://github.com/lollipop-onl/railsim2-portable/issues/9) (IME / CP932 input), [#5](https://github.com/lollipop-onl/railsim2-portable/issues/5) (texture upload)
- **Tree**: game `*.cpp` / `lib/*.cpp` at this document's commit. Counts exclude comments unless noted.
- **This slice does not implement FreeType, software buffers, or texture upload.** Later `#16` slices replace **this closed set**, not walk the whole tree.

## Corrections to issue #70 / parent #16

| Claim | Fact |
|-------|------|
| `GetTextExtentPoint32` / `GetTextExtentPoint` in the font path | **No game/`lib/` source calls either API.** Metrics use `DrawText` with `DT_CALCRECT` in `lib/texture.cpp` (`DrawInText`, `CalcTextRect`). |
| `CreateFont` always means Win32 GDI | **Name collision.** Win32 `CreateFont` (returns `HFONT`) appears in `RailSim2.cpp` and `CSkinPlugin.cpp`. A separate **`lib/font.cpp` `CreateFont(int, D3DCOLOR, LONG)`** wraps `D3DXCreateFontIndirect` (D3DX8, not GDI). |
| `CPixelbit*` is widely used outside capture | **Live `CPixelbit` callers today are essentially `Capture.cpp` only.** `SystemCover.cpp` and `CSceneryMode.cpp` include `CPixelbit.h` but call no `CPixelbit` API. |
| `InitExtFunc` / `msimg32.dll` is on the startup path | **`CPixelbit::InitExtFunc` has zero callers.** `TransparentBlt` is resolved lazily but never initialized in this tree. |
| `PasteFromClipboard` / `CopyToClipboard` / `WindowStamp` are in use | **Zero live callers** outside their defining TUs. |
| `CPixelbitDraw.cpp` / `CreateAAText` | Declared in `CPixelbit.h` but **no `CPixelbitDraw.cpp` in this tree** (not in `port/native_sources.txt`). Dead API until that TU lands. |
| `Capture.cpp` `BitBlt` belongs to `#16` | **Enumerate only here; implementation / no-op is `#17`.** |

## Architecture

```
Win32 HFONT                          CStringTexture (game)
-----------                          --------------------
RailSim2.cpp g_TempFont  ÑüÑüÑ¢
CSkinPlugin m_hFont      ÑüÑüÑ©ÑüÑü> g_StrTex->SetFont(HFONT)
                           Ñ†
                           v
                    DrawString / Enable
                           Ñ†
                           v
              CalcTextRect (lib/texture.cpp, GDI)
                           Ñ†
                           v
              CTexture::DrawInText (GDI DrawText -> DIB -> LockRect)
                           Ñ†
                           v
              CStringDrawer::Render* (D3D TexMap2DRect / 3D variants)

CPixelbit DIB buffer (Capture.cpp only on live path)
----------------------------------------------------
GetDC(svw.hWnd) -> BitBlt -> g_ScreenShot.GetHDC()     [#17 implements]
g_ScreenShot -> FastDownSample (software) -> Save / AVI
hidef_horz/vert -> BilinearStamp -> g_ScreenShot       [optional F12 quality]
saving_img -> PlainStamp -> g_Video24bit               [video record path]

Parallel (not GDI HFONT; note for M4 edit overlay)
--------------------------------------------------
lib/font.cpp CreateFont -> D3DXCreateFontIndirect -> Text() / TextF()
  callers: lib/editbox.cpp (live edit caret), lib/view_ctrl.cpp (debug HUD)
```

Game UI text almost never calls GDI directly. It goes through **`g_StrTex`** (`CStringTexture`) and the **`CTexture::DrawInText`** helper in `lib/texture.cpp`. `#16` replaces that raster seam plus the two Win32 `HFONT` creation sites.

## Closed GDI set ? direct Win32 / GDI API sites

### DIB / DC lifecycle (`CPixelbit.cpp`)

| API | Function | Role |
|-----|----------|------|
| `CreateDIBSection` | `CPixelbit::Clear` | Owns `m_BmpHnd`, `m_PixelAdr`, `m_BmpHdc` |
| `GetDC(0)` / `ReleaseDC` | `Clear` | Screen DC to seed compatible DC |
| `CreateCompatibleDC` | `Clear` | Off-screen `m_BmpHdc` |
| `SelectObject` | `Clear`, dtor | Select / restore bitmap into DC |
| `DeleteObject` | dtor, `Clear` (realloc) | Free prior `HBITMAP` |
| `DeleteDC` | dtor, `Clear` (realloc) | Free prior `HDC` |

`CPixelbit::Load` / `Save` / `Assign` / palette helpers operate on the DIB buffer or files; they do not add GDI calls beyond what `Clear` already establishes.

### Bit block transfer

| API | File | Lines | Context | Live callers |
|-----|------|-------|---------|--------------|
| `BitBlt` | `Capture.cpp` | 234, 252 | Window DC Å® `g_ScreenShot` HDC, `SRCCOPY` | F12 screenshot + video frame grab (**`#17`**) |
| `StretchBlt` | `CPixelbitStamp.cpp` | 181?182 | `StretchStamp` only | **zero external callers** |
| `SetStretchBltMode` | `CPixelbitStamp.cpp` | 181 | `HALFTONE` / `COLORONCOLOR` before `StretchBlt` | same |
| `SetDIBitsToDevice` | `CPixelbitStamp.cpp` | 94?95, 111?112 | `WindowStamp`, `PlainStamp` when `m_ColorKey==0xffffffff` | `PlainStamp` from `Capture.cpp` 262 (default color key) |
| `TransparentBlt` | `CPixelbitStamp.cpp` | 96, 113 | via `ms_TransBlt` when color key set | **no live path** (`InitExtFunc` never called; default key is `0xffffffff`) |
| `LoadLibrary("msimg32.dll")` / `GetProcAddress(..., "TransparentBlt")` | `CPixelbit.cpp` | 15?17 | `InitExtFunc` | **zero callers** |
| `FreeLibrary` | `CPixelbit.cpp` | 27 | `FreeExtFunc` | **zero callers** |

### Font creation / metrics / raster (HFONT path)

| API | File | Lines | Role |
|-----|------|-------|------|
| `CreateFont` (Win32) | `RailSim2.cpp` | 160?163 | Bootstrap `g_TempFont` before skin load |
| `CreateFont` (Win32) | `CSkinPlugin.cpp` | 202?205 | Per-skin `m_InterfaceData.m_hFont` from `FontName` script field |
| `DeleteObject` | `RailSim2.cpp` | 312 | Free `g_TempFont` after plugin load |
| `DeleteObject` | `CSkinPlugin.cpp` | 39 | Dtor frees skin font |
| `CreateCompatibleDC` | `lib/texture.cpp` | 99, 377 | Scratch DC for text |
| `SelectObject` | `lib/texture.cpp` | 101, 138, 205?206, 378, 384 | Font + DIB into scratch DC |
| `SetMapMode` | `lib/texture.cpp` | 103, 379 | `MM_TEXT` |
| `SetBkMode` | `lib/texture.cpp` | 104 | `TRANSPARENT` (DrawInText only) |
| `SetTextColor` | `lib/texture.cpp` | 105 | White mask for alpha extraction |
| `DrawText` + `DT_CALCRECT` | `lib/texture.cpp` | 111, 381 | Measure string (`DrawInText`, `CalcTextRect`) |
| `DrawText` | `lib/texture.cpp` | 140 | Rasterize into 32 bpp DIB |
| `CreateDIBSection` | `lib/texture.cpp` | 132?134 | Text bitmap for `DrawInText` |
| `DeleteObject` | `lib/texture.cpp` | 207 | Temporary text bitmap |
| `DeleteDC` | `lib/texture.cpp` | 208, 385 | Scratch DC teardown |

Stub only: `port/stub/windows.h` (`CreateFont`, `BitBlt`, `StretchBlt`, `TransparentBlt`, clipboard helpers).

### Clipboard

| API | File | Function | Format | Live callers |
|-----|------|----------|--------|--------------|
| `OpenClipboard` / `CloseClipboard` | `lib/editbox.cpp` | `ClipCopy`, `ClipPaste` | ? | Edit controls (cut/copy/paste) |
| `EmptyClipboard` | `lib/editbox.cpp` | `ClipCopy` | ? | same |
| `SetClipboardData` | `lib/editbox.cpp` | `ClipCopy` | `CF_TEXT` | same |
| `GetClipboardData` | `lib/editbox.cpp` | `ClipPaste` | `CF_TEXT` | same |
| `GlobalAlloc` / `GlobalLock` / `GlobalUnlock` | `lib/editbox.cpp` | `ClipCopy`, `ClipPaste` | text buffer | same |
| `OpenClipboard` Åc `GetClipboardData(CF_DIB)` | `CPixelbit.cpp` | `PasteFromClipboard` | `CF_DIB` | **zero callers** |
| `GlobalAlloc` Åc `SetClipboardData(CF_DIB)` | `CPixelbit.cpp` | `CopyToClipboard` | `CF_DIB` | **zero callers** |

## `CPixelbit` public API inventory

Compiled TUs today: `CPixelbit.cpp`, `CPixelbitStamp.cpp` (not yet on `port/native_sources.txt`; `CStringTexture.cpp` is).

| Category | Methods | Internal GDI / Win32 | Live external use |
|----------|---------|----------------------|-------------------|
| Lifecycle | ctor, dtor, `Clear`, `GetHDC`, pixel accessors | `CreateDIBSection`, DC ops above | `Capture.cpp` globals + locals |
| File I/O | `Load`, `Save` | none beyond `Clear` | `Capture.cpp` `Save` (BMP export) |
| Clipboard | `PasteFromClipboard`, `CopyToClipboard` | clipboard + `Global*` | **none** |
| Assign / convert | `Assign`, `Set4PAL*`, `Set4BGR*`, `PrepareDIB` | none | **none** (used internally) |
| Stamp | `PlainStamp`, `PlainStamp32`, `AlphaBlendStamp`, `StretchStamp`, `BilinearStamp`, `NearestStamp`, `WindowStamp`, `FixLocation`, `ResetAlphaChannel` | `SetDIBitsToDevice`, `StretchBlt`, `TransparentBlt` | **`PlainStamp`, `BilinearStamp` in `Capture.cpp` only** |
| Draw / FX / PNG (header only) | `CreateAAText`, `DrawRect`, `LoadPNG`, Åc | would add GDI/text ops | **TU not in tree** |
| Static | `InitExtFunc`, `FreeExtFunc` | `LoadLibrary(msimg32)` | **none** |

`FastDownSample` in `Capture.cpp` is **software** (reads `GetScanLine`); not part of the GDI op set.

## `CStringTexture` public API inventory

| Type | Method | GDI touchpoint | Role |
|------|--------|----------------|------|
| `CStringTexture` | ctor / dtor | none | Allocates atlas pages (`CTexture` 512Å~512 or 256Å~256 with `-voodoo`) |
| | `SetFont(HFONT)` | stores handle only | Switched at skin preview (`CSkinPlugin::SetPreview`) |
| | `DrawString` | `CalcTextRect` Å® `DrawInText` | Cache lookup + raster into atlas |
| | `Render*` / `Render*S` / `Render*V` / `Render*3D` | none (D3D draw) | Blit cached glyph rects |
| `CStringDrawer` | `Enable` | `DrawInText` when `w && h` | Writes glyph into atlas cell |
| | `Check`, `FindNewest`, `Reset`, `Init` | none | Cache management |
| | `Render*` | none | UV map + `TexMap2DRect` / 3D |

Global: `g_StrTex` (`RailSim2.cpp`), configured from `g_TempFont` then skin `m_hFont`.

### `g_StrTex` live caller files (34 TUs)

All use D3D render helpers after atlas upload; raster happens only inside `DrawString` Å® `DrawInText`.

`CCamera.cpp`, `CCheckBox.cpp`, `CConfigMode.cpp`, `CDiaDialog.cpp`, `CEditCtrl.cpp`, `CGameMode.cpp`, `CGroupBox.cpp`, `CInterfaceMode.cpp`, `CJobTimer.cpp`, `CListView.cpp`, `CMultiStatic.cpp`, `CPopMenu.cpp`, `CPushButton.cpp`, `CRadioButton.cpp`, `CRailBuildMode.cpp`, `CRailBuilder.cpp`, `CRailConnector.cpp`, `CRailEditMode.cpp`, `CRailPlanCurve.cpp`, `CSceneryMode.cpp`, `CSimpleDialog.cpp`, `CSkinPlugin.cpp`, `CStaticCtrl.cpp`, `CStructBuildMode.cpp`, `CStructEditMode.cpp`, `CToggleIcon.cpp`, `CTrainGroup.cpp`, `CTreeDirElement.cpp`, `CTreeFileElement.cpp`, `CWindowCtrl.cpp`, `GraphicCover.cpp`, `Network.cpp`, `RailMap.cpp`, `RailSim2.cpp`.

## Win32 `HFONT` vs `lib/font.cpp` (adjacent, out of closed GDI set)

| Symbol | File | Mechanism | Callers | `#16` note |
|--------|------|-----------|---------|------------|
| Win32 `CreateFont` Å® `HFONT` | `RailSim2.cpp`, `CSkinPlugin.cpp` | GDI font object | feeds `g_StrTex` | **In closed set** |
| `CreateFont(int, D3DCOLOR, LONG)` | `lib/font.cpp` | `D3DXCreateFontIndirect` | `lib/graphic.cpp` init / reset | D3DX8 backend seam; replace with same FreeType face or fold into `DrawInText` |
| `Text` / `TextF` | `lib/font.cpp` | `ID3DXFont::DrawTextA` | `lib/editbox.cpp`, `lib/view_ctrl.cpp` | Live IME composition + debug HUD; not HFONT/GDI |

Do not conflate the two `CreateFont` names when wiring stubs or port headers.

## M4 Sample vs deferrable

M4 goal (parent `#16`): **readable Japanese UI in single-player Sample**, not pixel-perfect GDI parity.

| Path | Classification | Rationale |
|------|----------------|-----------|
| `g_StrTex` + `DrawInText` + `CalcTextRect` + skin/RailSim2 `HFONT` | **M4 must** | Every mode label, button, list, tree, HUD, dialog skin text |
| `lib/editbox.cpp` `CF_TEXT` clipboard | **M4 must** | Cut/copy/paste in text fields (config, naming, search) |
| `lib/editbox.cpp` `Text()` via D3DX | **M4 must** (adjacent) | Live caret / IME composition display during edit |
| `CPixelbit` DIB buffer (`Clear`, scanline access) | **M4 must** if capture stays enabled | Underlies screenshot buffer even before `BitBlt` swap |
| `Capture.cpp` `BitBlt` | **Defer `#17`** | Optional feature; enumerate here, no-op or backend there |
| `HidefCapture` + `BilinearStamp` | **Defer** | High-quality F12 only (`g_HidefQuality>1`); not core Sample UI |
| Video `PlainStamp` / `Save` / AVI BMP path | **Defer** | Video record mode; not required for playable Sample |
| `CPixelbit` DIB clipboard (`CF_DIB`) | **Defer** | Zero callers |
| `WindowStamp`, `StretchStamp`, `TransparentBlt`, `InitExtFunc` | **Defer** | Zero live callers with current color-key defaults |
| `CPixelbitDraw` / `CreateAAText` | **Defer** | Source not compiled |
| `lib/view_ctrl.cpp` debug `Text()` | **Defer** | Debug overlay only |

## `Capture.cpp` `BitBlt` sites (enumeration for `#17`)

| Line | Trigger | Source DC | Dest | Notes |
|------|---------|-----------|------|-------|
| 234 | F12 screenshot (non-hidef branch) | `GetDC(svw.hWnd)` | `g_ScreenShot.GetHDC()` | Then `Save` BMP under `Picture/` |
| 252 | Video capture frame tick | same | same | Then downsample / `PlainStamp` / AVI or frame BMP |

Both paired with `ReleaseDC(svw.hWnd, windc)`. `#76` no-ops this readback (`VideoCapture` / `HidefCapture` return immediately). `#16` still owns the `CPixelbit` DIB layout; see [capture-seams.md](capture-seams.md).

## Out of scope (this slice / `#70`)

- FreeType / software raster / texture upload implementation
- `Capture.cpp` `BitBlt` backend (#17)
- CP932 / IME conversion (#9)
- Sync dialog wrappers (#12)
- Growing `port/native_sources.txt` or stubs beyond what `./scripts/check.sh` already requires

## `#16` contract (summary)

Replace **this closed GDI set**:

1. **Text atlas path** ? Win32 `HFONT` creation (`RailSim2.cpp`, `CSkinPlugin.cpp`), `CalcTextRect`, `CTexture::DrawInText` (GDI `DrawText` + DIB + upload).
2. **`CPixelbit` buffer** ? DIB section + DC lifecycle and stamp ops actually reached from `Capture.cpp` (`PlainStamp`, `BilinearStamp`, scanline software paths).
3. **Clipboard** ? `lib/editbox.cpp` `CF_TEXT` quartet (decide SDL clipboard vs initial no-op).
4. **Enumerate-only** ? `Capture.cpp` `BitBlt` (hand off to `#17`).

After `#16` planning, a reader should be able to answer: Ågimplement these APIs and every remaining GDI touch in the UI/capture path is accounted for.Åh
