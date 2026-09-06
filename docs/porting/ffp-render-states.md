# D3D8 fixed-function render state / FVF inventory

- **Issue**: [#63](https://github.com/lollipop-onl/railsim2-portable/issues/63) (parent [#5](https://github.com/lollipop-onl/railsim2-portable/issues/5))
- **Scope**: docs only. No shader, VB, texture, or offscreen implementation.
- **Tree**: game `*.cpp` / `lib/*.cpp` at this document's commit. Counts exclude comments and `port/stub/`.
- **Related**: [adr-backend.md](adr-backend.md) (FFP emulation strategy), [api-surface.md](api-surface.md) (raw `sv3.pDev` leak set).

## Executive summary

RailSim2 uses the **D3D8 fixed-function pipeline only**. Every `SetVertexShader` call passes an **FVF** (`DWORD`), not a programmable shader object. Game code sets render state through thin wrappers in `lib/render.h` (`devSetState` → `SetRenderState`) and `lib/texture.h` (`devSetTexState` → `SetTextureStageState`). Seven game files also call `sv3.pDev` directly for `SetVertexShader`, `SetTransform`, and draw calls ([api-surface.md](api-surface.md)).

This document closes the **used** sets of `D3DRS_*`, `D3DTSS_*`, FVF aliases, and transform types so M3 FFP emulation can implement exactly this surface and fail loudly on anything else. Parent #5 allows deferring **shadow volumes**, **lens flare**, and **particles**; their extra states are listed separately but remain part of the full closed set for a later slice.

**Not used anywhere in game/lib** (stub-only today): `D3DRS_TEXTUREFACTOR`, `D3DRS_COLORVERTEX`, `D3DRS_FOGDENSITY`. **Defined in headers but never called**: `devBLEND_NONE`, `devBLEND_RGB`, `devBLEND_ADD`, `devBLEND_MOD`, `devALPHA_GREAT`, `devALPHA_PASS`, `devTEX_SINGLE`, `devTEX_LIGHTMAP`, `devTEX_ENVMAP`, `devTEX_NONE`, `devTEX_WRAP` / `MIRROR` / `CLAMP` / `BORDER` / `MIRROR2`. Fog helpers exist but **no live path enables fog** (`devSetFog` / `devSetPixelFog` are only called with `FALSE` in `InitRenderState`; `CShadowVolume::Draw` also forces `FOGENABLE` off).

---

## API entry points

| Mechanism | Wrapper | Underlying D3D8 call | Primary files |
|-----------|---------|----------------------|---------------|
| Render state | `devSetState(type, value)` | `SetRenderState` | `lib/render.h`; direct callers in game + `lib/mesh.cpp` |
| Render state (typed) | `devSetLighting`, `devSetZRead`, `devSetBlend`, … | `SetRenderState` | `lib/render.h`; widespread game/lib |
| Texture stage | `devSetTexState(stage, type, value)` | `SetTextureStageState` | `lib/texture.h`; `lib/mesh.cpp`, `CShadowVolume.cpp` |
| Texture stage (typed) | `devSetTexColor`, `devSetTexFilter`, `devSetEnvMap`, … | `SetTextureStageState` + `SetTransform` | `lib/texture.h`, `lib/graphic.cpp`, `lib/mesh.cpp` |
| FVF | `sv3.pDev->SetVertexShader(fvf)` | `SetVertexShader` (FVF) | `lib/vertex.cpp`, `lib/draw.cpp`, `CVertexDump.cpp`, `CShadowVolume.cpp`, `RailSim2.cpp` |
| Transform | `sv3.pDev->SetTransform(type, &mtx)` | `SetTransform` | `lib/graphic.cpp`, `lib/graphic.h`, `lib/texture.h`, `CCamera.cpp`, `CVertexDump.cpp` |

`.x` mesh draw (`CMesh::Render` / `DrawSubset`) does **not** call `SetVertexShader`; D3DX owns the mesh FVF internally. `GetFVF()` is used only for bounding-box math (`lib/mesh.cpp`) and shadow-volume extraction (`CShadowVolume.cpp`).

---

## Closed `D3DRS_*` set

Values below are the **only** enumerants passed at live call sites. Wrappers may read back state (`devGetState`) for `D3DRS_LIGHTING`, `D3DRS_ZENABLE`, `D3DRS_ZWRITEENABLE` only.

### Core pipeline (M3 required)

| State | Values used | Set by | Notes |
|-------|-------------|--------|-------|
| `D3DRS_LIGHTING` | `TRUE` / `FALSE` | `devSetLighting` | Toggled per overlay (UI, map, editor, particles restore, etc.) |
| `D3DRS_AMBIENT` | `D3DCOLOR` | `devSetAmbient` | Scene ambient; `CEnvPlugin` drives day/night |
| `D3DRS_SPECULARENABLE` | `TRUE` / `FALSE` | `devSetSpecular` | Default on in `InitRenderState`; config UI toggles |
| `D3DRS_CULLMODE` | `D3DCULL_CCW`, `D3DCULL_NONE` | `devSetCulling` | CCW when culling on; NONE when off |
| `D3DRS_SHADEMODE` | `D3DSHADE_GOURAUD` | `devSetShading` | Default in `InitRenderState` |
| `D3DRS_NORMALIZENORMALS` | `TRUE` | `devSetNormalize` | Always on after init |
| `D3DRS_ZENABLE` | `TRUE` / `FALSE` | `devSetZRead` | Depth test on/off |
| `D3DRS_ZWRITEENABLE` | `TRUE` / `FALSE` | `devSetZWrite` | Depth write on/off |
| `D3DRS_ZFUNC` | `D3DCMP_LESSEQUAL`, `D3DCMP_ALWAYS` | `devSetState` | `ALWAYS` for decal-style draws (`CRailWay`, `CPartsInst`); default otherwise `LESSEQUAL` |
| `D3DRS_ALPHATESTENABLE` | `TRUE` / `FALSE` | `devSetAlphaTest`, `devSetState` | Mesh *alpha-zero* pass (`lib/mesh.cpp` MatFlag `0x20`) |
| `D3DRS_ALPHAFUNC` | `D3DCMP_GREATER`, `D3DCMP_ALWAYS` | same | `GREATER` + ref `0x00` for zero-alpha discard |
| `D3DRS_ALPHAREF` | `0x00` | `devSetState` | With `D3DCMP_GREATER` only |
| `D3DRS_ALPHABLENDENABLE` | `TRUE` / `FALSE` | `devSetBlend`, `devBLEND_*` | Default on (alpha blend) after init |
| `D3DRS_SRCBLEND` | `D3DBLEND_SRCALPHA` | `devSetBlend`, `devBLEND_ALPHA`, `devBLEND_ADD2` | See blend table below |
| `D3DRS_DESTBLEND` | `D3DBLEND_INVSRCALPHA`, `D3DBLEND_ONE` | same | `ADD2` uses dest `ONE` |
| `D3DRS_DIFFUSEMATERIALSOURCE` | `D3DMCS_COLOR1`, `D3DMCS_MATERIAL` | `devSetState` | Toggle per-object / per-plugin material path |
| `D3DRS_AMBIENTMATERIALSOURCE` | `D3DMCS_COLOR1`, `D3DMCS_MATERIAL` | `devSetState` | Paired with diffuse source |
| `D3DRS_FOGENABLE` | `FALSE` | `devSetFog`, `devSetPixelFog` | API present; **never enabled** at runtime |
| `D3DRS_FOGCOLOR` | ? | wrappers | Only if fog flag true (unused live) |
| `D3DRS_FOGVERTEXMODE` | `D3DFOG_LINEAR` | `devSetFog` | Unused live |
| `D3DRS_FOGTABLEMODE` | `D3DFOG_LINEAR` | `devSetPixelFog` | Unused live |
| `D3DRS_FOGSTART`, `D3DRS_FOGEND` | `float` as `DWORD` | fog wrappers | Unused live |

### Blend modes (`devBLEND_*` macros)

| Macro | `SRCBLEND` | `DESTBLEND` | Live callers |
|-------|------------|-------------|--------------|
| (init) | `SRCALPHA` | `INVSRCALPHA` | `InitRenderState`, `devBLEND_ALPHA` restore |
| `devBLEND_ALPHA()` | `SRCALPHA` | `INVSRCALPHA` | UI, config, particles restore, lens flare restore, `CNamedObject`, … |
| `devBLEND_ADD2()` | `SRCALPHA` | `ONE` | Opening logo, sun/moon billboards, lens flare, particles (add mode), misc effects |

Macros **`devBLEND_NONE`**, **`devBLEND_RGB`**, **`devBLEND_ADD`**, **`devBLEND_MOD`** are defined in `render.h` but have **zero** call sites.

### Shadow-only render states (M3 deferrable ? `CShadowVolume.cpp`)

| State | Values | Phase |
|-------|--------|-------|
| `D3DRS_STENCILENABLE` | `TRUE` / `FALSE` | Volume pass + darkening pass |
| `D3DRS_STENCILFUNC` | `D3DCMP_ALWAYS`, `D3DCMP_LESSEQUAL` | Build vs apply |
| `D3DRS_STENCILPASS` | `INCR`, `DECR`, `KEEP` | Volume build / apply |
| `D3DRS_STENCILZFAIL`, `D3DRS_STENCILFAIL` | `KEEP` | Volume build |
| `D3DRS_STENCILREF` | `0x1` | Both passes |
| `D3DRS_STENCILMASK`, `D3DRS_STENCILWRITEMASK` | `0xffffffff` | Volume build |
| `D3DRS_SHADEMODE` | `D3DSHADE_FLAT` | Volume build (restored to `GOURAUD`) |
| `D3DRS_CULLMODE` | `D3DCULL_CW` | Back faces during volume (restored to `CCW`) |
| `D3DRS_ZWRITEENABLE` | `FALSE` then `TRUE` | Volume vs restore |
| `D3DRS_ALPHABLENDENABLE` + blends | `ZERO`/`ONE` (volume), `SRCALPHA`/`INVSRCALPHA` (apply) | Stencil-only write, then fullscreen darken |
| `D3DRS_ZENABLE` | `FALSE` during apply | Fullscreen shadow quad |
| `D3DRS_FOGENABLE` | `FALSE` | Apply pass |

Game files touching material-source or `ZFUNC` (all M3 required): `RailSim2.cpp`, `CSaveFile.cpp`, `CScene.cpp`, `CRailPlugin.cpp`, `C3DPluginMode.cpp`, `CRailWay.cpp`, `CPartsInst.cpp`, `Capture.cpp`, `CSceneryMode.cpp`, `CEnvPlugin.cpp`.

---

## Closed `D3DTSS_*` / stage-op set

Stages **0** and **1** are used. Stage 1 is active only during env-map draws (`CMesh::RenderCustom`, MatFlag `0x08`).

### Stage 0 (M3 required)

| `D3DTSS_*` | Values | Set by |
|------------|--------|--------|
| `COLOROP` | `D3DTOP_MODULATE` | `devSetTexColor`, `InitRenderState`, shadow apply |
| `COLORARG1` | `D3DTA_TEXTURE` | same |
| `COLORARG2` | `D3DTA_DIFFUSE` | same |
| `ALPHAOP` | `D3DTOP_MODULATE` | `devSetTexAlpha`, `InitRenderState`, shadow apply |
| `ALPHAARG1` | `D3DTA_TEXTURE` | same |
| `ALPHAARG2` | `D3DTA_DIFFUSE` | same |
| `MAGFILTER`, `MINFILTER`, `MIPFILTER` | `D3DTEXF_POINT`, `D3DTEXF_LINEAR` | `devSetTexFilter`, `devTEX_POINT`, `devTEX_LINEAR`, config UI |
| `TEXTURETRANSFORMFLAGS` | `D3DTTFF_DISABLE`, `D3DTTFF_COUNT2` | `devSetTexTrans` (MatFlag `0x10` UV transform) |
| `TEXCOORDINDEX` | `D3DTSS_TCI_PASSTHRU`, `D3DTSS_TCI_CAMERASPACENORMAL` | `devSetEnvMap` on stage 1; stage 0 stays passthrough |

Address modes (`ADDRESSU`/`V`): wrappers exist (`devTEX_WRAP`, …) but **no live caller** ? rely on D3D defaults (wrap).

### Stage 1 (M3 required when env-map materials present)

| `D3DTSS_*` | Values | When |
|------------|--------|------|
| `COLOROP` | `D3DTOP_MODULATE`, `D3DTOP_DISABLE`, `D3DTOP_ADDSMOOTH` | Env map on (`MODULATE` + `CURRENT`); off restores `DISABLE` |
| `COLORARG1` | `D3DTA_TEXTURE`, `D3DTA_CURRENT` | Env map path |
| `COLORARG2` | `D3DTA_CURRENT`, `D3DTA_DIFFUSE` | Env map / disable |
| `MAG/MIN/MIPFILTER` | `D3DTEXF_POINT` | Paired with stage 0 during alpha-zero test |
| `TEXTURETRANSFORMFLAGS` | `D3DTTFF_COUNT2` | Via `devSetEnvMap(1, TRUE)` |
| `TEXCOORDINDEX` | `D3DTSS_TCI_CAMERASPACENORMAL` | Via `devSetEnvMap` |

`devSetEnvMap` also sets `D3DTS_TEXTURE1` to a fixed camera-space normal matrix (see transforms).

Shadow apply pass sets stage-0 ops explicitly (same as init defaults) ? counted under deferrable shadow slice.

---

## Closed FVF set

All FVF aliases live in `lib/vertex.h` except `FVF_S` in `CShadowVolume.h`. `SetVertexShader` always receives one of these **named** constants (never a runtime-computed FVF except `.x` mesh internals).

| Alias | `D3DFVF_*` bits | Struct | Role | M3 tier |
|-------|-----------------|--------|------|---------|
| `FVF_TL` | `XYZRHW \| DIFFUSE` | `VTX_TL` | 2D lines, UI fills, editor dumps | **required** |
| `FVF_TLX` | `XYZRHW \| DIFFUSE \| TEX1` | `VTX_TLX` | Textured 2D (opening, fonts, sprites) | **required** |
| `FVF_L` | `XYZ \| DIFFUSE` | `VTX_L` | Pre-lit 3D lines / grids | **required** |
| `FVF_LX` | `XYZ \| DIFFUSE \| TEX1` | `VTX_LX` | Pre-lit textured quads (`lib/draw.cpp`) | **required** |
| `FVF_LX2` | `XYZ \| DIFFUSE \| TEX2` | `VTX_LX2` | Dual UV pre-lit (`lib/draw.cpp`) | **required** |
| `FVF_N` | `XYZ \| NORMAL \| DIFFUSE` | `VTX_N` | Lit lines / editor geometry | **required** |
| `FVF_NX` | `XYZ \| NORMAL \| DIFFUSE \| TEX1` | `VTX_NX` | Lit textured (`CVertexDump`, terrain helpers) | **required** |
| `FVF_NX2` | `XYZ \| NORMAL \| DIFFUSE \| TEX2` | `VTX_NX2` | Supported in `CVertex` only; no live `SetVertexShader` caller | **required** (VB path) |
| `FVF_S` | `XYZ` only | `VTX_S` | Shadow volume triangles | **deferrable** |

**Deferrable FVF note:** `lib/particle.cpp` builds `FVF_LX` buffers but `CParticle` (game) renders via `TexMap3DRect` / `FVF_TLX`-style immediate draws, not `CParticle::RenderAll` → `lib/particle.cpp`. Treat particle FVF needs as part of the deferrable effect slice.

Commented-out udx modules (`water_mesh`, `height_field`, `particle.h`) are **not** in the compile graph; their FVFs remain in the closed set for when those TUs re-enter the allowlist.

---

## Transform matrices (`SetTransform`)

| Type | Values | Set by | M3 tier |
|------|--------|--------|---------|
| `D3DTS_WORLD` | `sv3.mtxWorld`, object matrices | `lib/graphic.cpp`, `lib/graphic.h` (`devTransform`) | **required** |
| `D3DTS_VIEW` | `sv3.mtxView` | `lib/graphic.cpp`, `CCamera.cpp` | **required** |
| `D3DTS_PROJECTION` | `sv3.mtxProj`, per-cell bold-line override | `lib/graphic.cpp`, `CCamera.cpp`, `CVertexDump.cpp` | **required** |
| `D3DTS_TEXTURE0` + stage | UV transform matrix | `devTexTransform` (`lib/mesh.cpp` MatFlag `0x10`) | **required** |
| `D3DTS_TEXTURE0+1` | Fixed env-map matrix | `devSetEnvMap` | **required** (env materials) |

No other `D3DTS_*` types appear at call sites.

---

## Effect slices: M3 required vs deferrable

Parent #5 defers visual parity for **shadow**, **lens flare**, and **particles**. Split below.

| Feature | Key TU | States / FVF beyond core | Tier |
|---------|--------|--------------------------|------|
| Core scene + UI + meshes | `lib/graphic.cpp`, `CSaveFile.cpp`, `CScene.cpp`, plugins | Init table + material-source + dual-texture env + UV transform + alpha test | **M3 required** |
| Sun/moon billboards | `CEnvPlugin.cpp` | `devBLEND_ADD2`, Z off, lighting off (restored) | **M3 required** (sky) |
| Lens flare / whiteout | `CLensFlare.cpp`, `lib/effect.cpp`, `CEnvPlugin::RenderAfter` | `devBLEND_ADD2` ? `devBLEND_ALPHA`, Z write off, lighting off | **deferrable** |
| Particles | `CParticle.cpp` | Z write off, lighting off, per-sprite `devBLEND_ADD2` or `ALPHA` | **deferrable** |
| Headlight glow | `CLensFlare.cpp` (`CHeadlight`) | Same pattern as particles | **deferrable** |
| Shadow volumes | `CShadowVolume.cpp` | Full stencil block + `FVF_S` + flat shade + CW cull | **deferrable** |

Config gates: `SunLensFlare`, `MiscLensFlare`, `MiscParticle` (`CConfigMode`) ? effects skip entirely when unchecked.

---

## Default device state (`InitRenderState`)

`lib/graphic.cpp` establishes the baseline restored by most draw paths:

```357:375:lib/graphic.cpp
void InitRenderState(){
	devSetLighting(TRUE);
	devSetAmbient(0xff808080);
	devSetSpecular(TRUE);
	devSetShading(D3DSHADE_GOURAUD);
	devSetCulling(TRUE);
	devSetZRead(TRUE);
	devSetZWrite(TRUE);
	devSetFog(FALSE, 0, 0, 0);
	devSetPixelFog(FALSE, 0, 0, 0);
	devSetBlend(TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
	devSetNormalize(TRUE);

	devSetTexColor(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	devSetTexAlpha(0, D3DTOP_MODULATE, D3DTA_TEXTURE, D3DTA_DIFFUSE);
	devSetTexFilter(0, D3DTEXF_POINT);

	g_BufferClearMode = D3DCLEAR_ZBUFFER | (g_StencilEnabled ? D3DCLEAR_STENCIL : 0);
	sv3.pDev->Clear(0, NULL, D3DCLEAR_TARGET|g_BufferClearMode, 0, 1.0f, 0);
}
```

FFP emulation should treat this as the **reference snapshot** after device reset / frame begin.

---

## Verification

Re-run from repo root (expects stub excluded):

```bash
# Closed D3DRS tokens (game + lib)
rg -o 'D3DRS_[A-Z0-9_]+' --glob '*.{cpp,h}' --glob '!port/stub/**' | sort -u

# Closed D3DTSS / TOP / TA tokens
rg -o 'D3DTSS_[A-Z0-9_]+|D3DTOP_[A-Z0-9_]+|D3DTA_[A-Z0-9_]+' \
  --glob '*.{cpp,h}' --glob '!port/stub/**' | sort -u

# Closed FVF aliases
rg -o 'FVF_[A-Z0-9]+' --glob '*.{cpp,h}' --glob '!port/stub/**' | sort -u

# Direct SetVertexShader call sites (FVF)
rg -n 'SetVertexShader\(' --glob '*.{cpp,h}' --glob '!port/stub/**'
```

---

## Vertex layer (port, #74)

Stride, attribute offsets, CPU vertex buffers, and `DrawPrimitiveUP` ring records are in [`ffp-vertex-layer.md`](ffp-vertex-layer.md) (`port/ffp_fvf.*`). `FVF_S` stays rejected. GLSL / render-state emulation is a later `#5` slice.

## What #5 should implement next

1. **M3 required tier** ? Implement the core `D3DRS_*` / stage-0 / stage-1-env / FVF shader matrix rows above until `Distribution/jp/RailSim2/Layout/Sample.rs2` renders without shadow, flare, or particles. FVF stride / CPU VB / UP *record* already live in `port/ffp_fvf` (#74); wire `IDirect3DDevice8` and upload the ring to a dynamic VBO.
2. **Deferrable tier** ? Add stencil shadow pass, additive flare/particle blends, and `FVF_S` as separate slices after core parity.
3. **Do not expand** ? No new render states in game code without updating this document; unknown FVF/state combos should assert in debug builds ([adr-backend.md](adr-backend.md)).
