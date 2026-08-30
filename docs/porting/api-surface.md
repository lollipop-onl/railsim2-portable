# lib/ (udx) public API surface

This document freezes the **portable ABI** that game code already depends on.

If a later backend implements this surface, the 254 root-level game sources can stay unchanged and still compile. That is the M1 #1 exit condition.

Counts below are from the tree at this document's commit. Re-run the commands in [Verification](#verification) if the tree moves.

## Corrections to issue #1

Issue #1's background text is wrong on two points. The inventory here is the source of truth.

| Claim in #1 | Fact |
|-------------|------|
| DirectX / Win32 leaks from game in `RailMap.h` only | `RailMap.h` exposes `VEC2` / `VEC3` / `D3DCOLOR` in public signatures. That is a type leak, not the only isolation hole. |
| `D3DCOLOR` appears in 5 places and can be replaced with a private 32-bit color type | **43 game files, 217 uses.** Color is packed into vertices, UI, scripts, and draw helpers. Keep `D3DCOLOR` as `DWORD` ARGB. |
| Platform code is fully isolated in `lib/` | Almost. Game still pokes `sv3.pDev` in **7 files**, uses GDI directly (issue **#16**), and talks DirectPlay through `comm.h` (issue **#11**, optional). |

Do **not** rename D3D types to "portable" aliases in game sources. The port strategy is a D3D8-shaped device, mesh, and math surface behind `lib/` and `port/stub/`.

## Include graph

Game translation units enter udx through `stdafx.h`:

```9:9:stdafx.h
#include "lib/udx.h"
```

120 of 124 root `.cpp` files include `stdafx.h`. The four that do not are `CPixelbit.cpp`, `CPixelbitStamp.cpp`, `HighTimer.cpp`, and `md5.cpp`. Pixelbit / HighTimer include `<windows.h>` themselves (GDI / timer island).

`lib/udx.h` currently includes:

| Status | Headers |
|--------|---------|
| Active | `headers.h`, `debug.h`, `mutex.h`, `frame.h`, `window.h`, `app.h`, `graphic.h`, `view.h`, `render.h`, `light.h`, `vertex.h`, `draw.h`, `texture.h`, `font.h`, `mesh.h`, `object.h`, `anim.h`, `offscreen.h`, `effect.h`, `input.h`, `view_ctrl.h`, `editbox.h`, `sound.h`, `wave.h`, `wave_stream.h`, `comm.h` |
| Commented out | `sprite.h`, `water_mesh.h`, `height_field.h`, `particle.h`, `music.h`, `movie.h` |

`headers.h` is the DX / Win32 fan-in: `<d3d8.h>`, `<d3dx8.h>`, `<dinput.h>`, `<dmusicc.h>`, `<dmusici.h>`, `<dxfile.h>`, plus `<windows.h>` / `<imm.h>`. Game never includes those DX headers directly.

Typedefs that **are** the public math / resource names:

```48:62:lib/headers.h
typedef D3DXVECTOR2		VEC2;
typedef D3DXVECTOR3		VEC3;
typedef D3DXVECTOR4		VEC4;
typedef D3DXMATRIX		MTX4;
typedef D3DMATERIAL8	MAT8;
typedef D3DXQUATERNION	QUAT;

typedef LPDIRECT3DTEXTURE8 LPTEX8;
typedef LPDIRECT3DSURFACE8 LPSURF8;
typedef LPDIRECTSOUNDBUFFER		LPSNDBUF;
typedef LPDIRECTSOUNDBUFFER8	LPSNDBUF8;
typedef LPDIRECTSOUND3DBUFFER	LP3DBUF;
```

Game-side aliases in `Macro.h` / `Const.h` (`V3Norm`, `V3ZERO`, ?c) are macros over D3DX. They stay.

## Frozen type surface

These layouts are ABI. Backends may reimplement methods; they must keep field names and packing that game and `lib/` already use.

### Must stay D3D-shaped

| Type / macro | Why |
|--------------|-----|
| `D3DCOLOR` (`DWORD` ARGB) | 43 game files / 217 uses. Packed into `VTX_*`, draw APIs, `CStringTexture`, scripts. |
| `D3DCOLORVALUE` | Lights / materials (`MAKE_CV`, `SetDirLight`). |
| `VEC2` / `VEC3` | Dominant math. Game uses `.x/.y/.z` and D3DX overloads. `VEC3` is in 111 game files (~900 uses). |
| `MTX4` (`D3DXMATRIX` `_11`?c`_44`) | Camera, objects, `SetTransform`. |
| `MAT8` (`D3DMATERIAL8`) | Mesh materials. |
| `VTX_TL`, `VTX_TLX`, `VTX_L`, `VTX_N`, `VTX_NX` + `FVF_*` | Immediate-mode dumps and shadows. |
| `BOX8` | Object bounds. |
| `D3DPRESENT_PARAMETERS` as `sv3.d3dpp` | Capture reads format / size. |
| `D3DVIEWPORT8` | `CCamera.cpp` `GetViewport`. |
| Render-state enums used with `devSetState` | See [FFP states](#ffp-states-used-from-game). |
| `DIK_*`, `DIM_*`, `S_PUSH` / `S_HOLD` / `S_FREE` / `S_PULL` | Input. |
| `DPNID` | `Network.cpp` (M5 / #11; still needed to *compile*). |
| `POINT` | 39 game files (cursor / UI). Win32-shaped, not D3D, but frozen until a later input rewrite ? which we are not doing. |

`VEC4`, `QUAT`, `LPSURF8`, `LPSNDBUF*` are typedef'd and unused from game. Keep them for `lib/` anyway; do not delete from `headers.h`.

### May be opaque pointers (same typedef names)

`LPTEX8`, `LPDIRECT3DDEVICE8` (`sv3.pDev`), `LPD3DXMESH`, sound buffers. Game mostly passes them through. The 7 leak files call methods on `pDev`, so the **device vtable / method set below is not opaque**.

### Out of this freeze (do not rewrite for M1)

| Island | Issue | Files |
|--------|-------|-------|
| GDI (`HDC`, `HFONT`, `HBITMAP`, `BitBlt`, `CreateFont`) | **#16** | `CPixelbit*`, `Capture.cpp` screenshot path, `CSkinPlugin`, `CStringTexture`, `RailSim2.cpp` fonts |
| DirectPlay wire | **#11** | `Network.cpp` / `comm.h` ? compile stubs yes; protocol compatibility no |
| Capture AVI / `CopyRects` runtime | **#17** | `Capture.cpp` ? method must exist; body may no-op |
| Web | **#14** | Backend choice in #2 considers it; no WASM work in M1 |

## Frozen runtime API (called from game)

This is the "implement these and game links against udx" list. Init helpers (`InitDirect3D`, ?c) are called from `lib/main.cpp`, not from game, but they remain part of the lib contract.

### Scene / camera / device globals

- Globals: `sv3`, `svw`, `svi`, `svs`, `svf`, `svl`, `g_frame`, `g_MeshList`, `g_TexList`
- `BeginScene` / `EndScene` (udx wrappers). Some game files also call `sv3.pDev->EndScene` directly (see leaks).
- `SetView`, `SetViewport`, `GetVPos`, `GetVDir`, `GetVRight`, `GetVUp`, `GetVMatrix`, `LookAtV`, `WorldToScreen`
- `sv3.width` / `height` / `mtxProj` / `mtxFront` / `u,v` / `capsMaxPrim` / `fWindowed` / `d3dpp`

### Render state (udx wrappers)

Must exist with D3D8 semantics for the states actually passed:

- `devSetState` / `devGetState`
- `devSetLighting`, `devSetZRead`, `devSetZWrite`, `devResetMatrix`, `devTransform`
- `devSetTexture`, `devSetTexState`, `devSetTexFilter` and macros `devTEX_POINT`, `devTEX_LINEAR`, `devTEX_SINGLE`, `devBLEND_ALPHA`, `devBLEND_ADD`, ?c
- `devSetMaterial`, `SetDirLight`, `MAKE_CV`, `MAKE_AC` / `MAKE_XC`

Game does **not** call `devSetFog` / `devSetBlend` by those names; it uses the macros and raw `D3DRS_*` through `devSetState`.

### Draw helpers (`draw.h`)

Used from game (keep all of `draw.h`; these are the hot ones):

- `SetUVMap`
- `TexMap2DRect`, `Fill2DRect`, `Draw2DRect`, `Grad2DRect`, `Draw2DLine`
- `Draw3DLine`, `TexMap3DRect`, `DrawBox`

### Objects / meshes / textures / vertices

- `CObject`, `CMesh`, `CMeshList`, `CTexList`, `CTexture`, `CVertex`
- `CNamedObject` is a **game** class (`CNamedObject.h`). `lib/object.h` and `lib/mesh.h` forward-declare it. Keep that layering leak; do not move the class into `lib/` in M1.

### UI / text / IME

- `CEditBox` (game uses it; Imm* stays inside lib)
- `Text` / `TextF` / `BeginFont` are **unused from game**. Game text is `CStringTexture` + Win32 `HFONT` (GDI island #16). D3DX font in `lib/font.cpp` can stay as a lib-internal path.

### Input / window / dialogs

- `GetKey`, `GetButton`, `GetCursorXY`, `GetWheel`, `ScanInputDevice`, `FlushKey`, `DequeueChar`
- `SelectFile`, `Dialog`, `ErrorDialog`, `MsgBox` / `MsgYesNo`
- `PeekAllMessage`, `IsActive`, `svw.hWnd` (GDI / Capture)

### Sound

- `CWave` (via `CWaveArray` / `CSoundEffector`)
- Listener helpers used by game remain in `sound.h`

### Offscreen

- `COffScreen` (`g_HidefCapture` in Capture / Scene / Env)

### Comm (compile only in M1)

- `CreateSession`, `JoinSession`, `CloseSession`, `SendTo`, `SendToAll`, `IsHost`, `SetReceiveFunc`, `GetLocalPlayerID`

### Unused from game (keep in lib, not a game freeze)

`CAnim`, `CXFile`, `CApp`, `CMutex`, `CWaveStream`, `CFrame` as a type name (game uses `GetFPS` / `SyncFrame` / `MAXFPS`). Commented udx modules are unused from game (`lib/particle` is not game's `CParticle`).

## Isolation leaks (do not "fix" by editing game in M1)

### `sv3.pDev` ? 7 files

These are the complete set of game translation units that call through the raw device:

| File | Methods |
|------|---------|
| `CVertexDump.cpp` | `SetVertexShader`, `DrawPrimitiveUP`, `SetTransform` |
| `CShadowVolume.cpp` | `SetVertexShader`, `DrawPrimitiveUP` |
| `RailSim2.cpp` | `SetVertexShader`, `DrawPrimitiveUP` |
| `CSceneryMode.cpp` | `EndScene` |
| `CWindowDivInfo.cpp` | `EndScene` |
| `CCamera.cpp` | `GetViewport`, `SetTransform` |
| `Capture.cpp` | `CreateTexture`, `CopyRects` |

FVF used at those call sites: `FVF_TL`, `FVF_TLX`, `FVF_L`, `FVF_N`, `FVF_NX`, and local `FVF_S` (`D3DFVF_XYZ`) in `CShadowVolume`. Primitive types: `D3DPT_LINELIST`, `D3DPT_TRIANGLELIST`, `D3DPT_TRIANGLEFAN`.

**Implication:** `IDirect3DDevice8` in `port/stub/` (and later the real backend) must declare and implement at least:

`SetVertexShader`, `DrawPrimitiveUP`, `SetTransform`, `EndScene`, `GetViewport`, `CreateTexture`, `CopyRects`.

`SetVertexShader` here is **FVF**, not a programmable shader. 23 `SetVertexShader` calls in lib are the same pattern.

### `RailMap.h`

```4:8:RailMap.h
void DumpMapLine(VEC2, D3DCOLOR, VEC2, D3DCOLOR, bool shadow = true);
void RailMapLine(VEC3, D3DCOLOR, VEC3, D3DCOLOR, bool shadow = true, bool bold = false);
void RailMapText(VEC3, char *, D3DCOLOR);
void RenderRailMap();
```

No `#include <d3d8.h>`. Types arrive via `stdafx.h`. Isolation "violation" is the D3D-shaped public API, which we keep.

### Direct D3DX from game

Keep these as D3DX-named functions (implement in stub / glm-backed layer):

- `D3DXVec2Normalize` / `Length` / `Dot` (`Macro.h`)
- `D3DXVec3Normalize` / `Length` / `Dot` / `Cross` (`Macro.h`)
- `D3DXVec3TransformCoord`, `D3DXVec3TransformNormal`
- `D3DXMatrixPerspectiveFovLH`, `D3DXMatrixPerspectiveOffCenterLH`, `D3DXMatrixTranslation`
- `D3DXToRadian`, `D3DXToDegree`, `D3DX_PI`
- `D3DXGetFVFVertexSize` (`CShadowVolume.cpp`)

### GDI (issue #16)

Not an M1 rewrite. Compile-firewall stubs for `HDC` / `HFONT` / blit APIs are enough until that issue. Do not fold GDI into the D3D device contract.

## FFP states used from game

`devSetState(D3DRS_*, ?c)` from game (plus wrappers). This is the **minimum FFP emulation set** for #2 / #5. Shadows use stencil; materials use color-source states.

| State | Where (game) |
|-------|----------------|
| `D3DRS_STENCILENABLE`, `STENCILPASS`, `STENCILREF`, `STENCILFUNC`, `STENCILZFAIL`, `STENCILFAIL`, `STENCILWRITEMASK`, `STENCILMASK` | `CShadowVolume.cpp` |
| `D3DRS_ALPHABLENDENABLE`, `SRCBLEND`, `DESTBLEND`, `CULLMODE`, `SHADEMODE`, `FOGENABLE`, `ZENABLE`, `ZWRITEENABLE` | `CShadowVolume.cpp` |
| `D3DRS_DIFFUSEMATERIALSOURCE`, `D3DRS_AMBIENTMATERIALSOURCE` | `CSaveFile`, `CScene`, `CRailPlugin`, `C3DPluginMode`, `RailSim2` |
| `D3DRS_ZFUNC` | `CRailWay`, `CPartsInst`, `Capture`, `CSceneryMode`, `CEnvPlugin` |

Lib additionally drives fog, alpha test, texture-stage ops, lights, and transforms. Backend ADR (#2) must cover **lib + game** together; this table is only the game-side extras that bypass wrappers.

## Lib backend contract (what udx already calls)

`lib/` is the other half of the freeze. A real device must satisfy both game leaks and these `sv3.pDev` uses inside udx (highest counts): `SetVertexShader`, `SetMaterial`, `DrawPrimitiveUP`, `SetStreamSource`, `DrawPrimitive`, `SetTransform`, `Clear`, `SetViewport`, `SetRenderTarget`, `Reset`, `LightEnable`, `Present`, `BeginScene` / `EndScene`, `CreateVertexBuffer`, `CreateTexture`, `CopyRects`, `GetDeviceCaps`, ?c

D3DX used inside `lib/` (mesh / texture / math): `D3DXLoadMeshFromX` / `Xof`, `D3DXCreate{Box,Sphere,Teapot}`, `D3DXComputeBounding*`, `D3DXCreateTextureFromFileExA`, `D3DXCreateSprite`, `D3DXCreateFontIndirect`, matrix / quaternion helpers. M2 (#6) owns a closed `.x` parser + `ID3DXMESH`-like object. M1 only needs declarations so more TUs can compile.

## What #3 may grow from this

M0 `port/stub/` already sketches types and a silent `IDirect3DDevice8`. To move from object-only `HighTimer.cpp` toward a link skeleton:

1. Expand stubs until allowlisted TUs that include `stdafx.h` compile (math operators, missing D3D methods, Win32 leftovers).
2. Add a native executable *target* (link errors remaining are acceptable if the objects exist).
3. Do **not** wait for 254/254 before landing the skeleton. Growing `port/native_sources.txt` is monotonic progress; keep `./scripts/check.sh` green.

Full-source compile-through remains the issue #3 written exit. Treat it as the allowlist approaching 254, not as a requirement to ship a running binary in M1.

## Verification

From the repo root:

```bash
# D3DCOLOR leak size (expect ~43 files, ~217 hits)
rg -l 'D3DCOLOR' --glob '!lib/**' --glob '!port/**' --glob '!docs/**' --glob '!Distribution/**' | wc -l
rg -c 'D3DCOLOR' --glob '!lib/**' --glob '!port/**' --glob '!docs/**' --glob '!Distribution/**' | awk -F: '{s+=$2} END {print s}'

# Raw device pokes (expect the 7 files listed above)
rg -l 'sv3\.pDev' --glob '!lib/**' --glob '!port/**'

# Commented udx modules must not be included from game
rg -l '#include.*(sprite|water_mesh|height_field|particle|music|movie)\.h' --glob '!lib/**'
```

## Related

- Dev environment / compile firewall: [dev-env.md](dev-env.md)
- Upstream follow policy: [upstream.md](upstream.md)
- Backend choice: issue #2 (`docs/porting/adr-backend.md`, landed separately)
- Milestone: [M1 Surface](https://github.com/lollipop-onl/railsim2-portable/milestone/2)
