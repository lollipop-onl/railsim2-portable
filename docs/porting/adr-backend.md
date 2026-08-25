# ADR: native / web graphics backend

- **Status**: accepted
- **Issue**: [#2](https://github.com/lollipop-onl/railsim2-portable/issues/2)
- **Date**: 2026-08-25
- **Depends on**: frozen ABI in `docs/porting/api-surface.md` (issue #1 / PR #20)

## Decision

Use **one implementation** for macOS, Linux, and a later WebAssembly build:

| Layer | Choice |
|-------|--------|
| Window, events, GL context | **SDL2** |
| GPU API | **OpenGL ES 3.0 / WebGL2 subset**, expressed on desktop as **OpenGL 3.3 core** (macOS: 4.1 core, the last Apple GL) |
| Fixed-function pipeline | **Our own FFP + FVF shader family** in `lib/` / a small port backend. Not bgfx. Not Emscripten `-sLEGACY_GL_EMULATION`. |
| Audio | **OpenAL Soft** (3D listener / sources map to existing `CWave` / `svs`) |
| Text (later, #16) | **FreeType** replacing GDI/`HFONT`; not part of this ADR?fs runtime |
| Math | Keep **D3DX-shaped types** (`VEC3`, `MTX4`, ?c). Implement D3DX helpers in `port/stub/` (glm internally is allowed; the public names stay D3D) |

Web (#14) stays optional. The stack is chosen now so M3 bring-up does not paint us into a desktop-only GL 1.x / Metal / Vulkan corner.

## Context

RailSim2 draws with the **D3D8 fixed-function pipeline**. `SetVertexShader` appears in game and `lib/`, but every call site passes an **FVF**, not a shader object. There is no programmable D3D shader in this tree.

WebGL2 has no FFP. Desktop GL 3.3 core / GLES3 also have no FFP and no client-side arrays. Immediate `DrawPrimitiveUP` must become a streaming VBO on every target. That translation belongs in the **device implementation**, not in game code (see the seven `sv3.pDev` leak files in `api-surface.md`).

If native used a different GPU API than web (Metal, Vulkan, D3D11, or a desktop-only compatibility profile), FFP emulation would be written twice. That is the failure mode this ADR exists to avoid.

Emscripten?fs documented GL modes ([OpenGL support](https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html)):

1. WebGL-friendly GLES2/3 (recommended for new code)
2. `-sFULL_ES2` / `-sFULL_ES3` (client-array / map-buffer helpers)
3. `-sLEGACY_GL_EMULATION` (partial GL 1.x / FFP)

(3) is the wrong layer: incomplete, not shared with native, and not aligned with the **closed** FVF/state set we actually use.

SDL2 is the combination Emscripten and SDL document for a GLES3 context (`SDL_GL_CONTEXT_PROFILE_ES`, major 3) plus `-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2`. SDL3 is not required and is less proven on Emscripten for this project.

## FFP emulation scope (closed set)

Pixel-perfect is not a goal. **Used semantics are.** Do not invent extra texture ops ?gwhile playing.?h

### Vertex formats (must have shaders)

From game `DrawPrimitiveUP` / `CVertex` plus udx draw helpers:

| FVF | Typical use |
|-----|-------------|
| `FVF_TL` | 2D lines (`CVertexDump`) |
| `FVF_TLX` | Transformed textured fan (`RailSim2.cpp`) |
| `FVF_L` | Unlit colored lines |
| `FVF_N` | Lit colored |
| `FVF_NX` | Lit textured |
| `FVF_S` (`D3DFVF_XYZ` only) | Shadow volume |
| `FVF_LX` / `FVF_LX2` / `FVF_NX2` | udx `vertex.h` / 2D-3D helpers / lightmaps |

Implement these as a **small matrix of shader variants** (or ubershader with compile-time defines), selected by FVF + a packed render-state key. Unknown FVF / state combinations must fail loudly in debug, not silently look ?gclose enough.?h

### Render states that must mean something

Defaults from `InitRenderState()` (`lib/graphic.cpp`): lighting on, ambient `0xff808080`, specular on, Gouraud, CCW cull, Z read/write, alpha blend SRCALPHA/INVSRCALPHA, normalize normals, stage 0 MODULATE texture?~diffuse, point filter.

Plus states game sets through `devSetState` (see `api-surface.md`):

- Stencil set used by `CShadowVolume`
- `D3DRS_DIFFUSEMATERIALSOURCE` / `AMBIENTMATERIALSOURCE`
- `D3DRS_ZFUNC`
- Fog enable/color/start/end (vertex and table modes exist in udx; emulate linear fog first)
- Alpha test (`lib/mesh.cpp`)
- Texture stage 0?1: `MODULATE`, `ADDSMOOTH`, `DISABLE`, env-map texgen (`devSetEnvMap`)

Lights: one directional light (`SetDirLight`) is the gameplay path. Caps fields on `sv3` can be honest constants.

### Device methods the GL backend must honor

Wrappers in `render.h` / `texture.h` **and** raw `sv3.pDev` from the seven leak files: `SetVertexShader` (FVF), `DrawPrimitiveUP`, `DrawPrimitive`, `SetStreamSource`, `SetTransform`, `SetMaterial`, `SetTexture` / stage state, `Clear`, `BeginScene`/`EndScene`/`Present`, `SetViewport`, `SetRenderTarget`, `CreateTexture`, `CopyRects` (no-op until #17), `GetDeviceCaps`, `Reset`.

`.x` mesh load is M2 (#6): `ID3DXMESH`-like + a closed parser. This ADR only requires that the **device** can draw the FVF buffers that parser will produce.

## Alternatives considered

| Option | Why not |
|--------|---------|
| **bgfx / sokol / wgpu-native** | Extra abstraction. Web path still needs a GLES/WebGL or wgpu backend; FFP would be written against *their* model, then mapped again. Cost without helping the D3D8 method surface. |
| **GLFW + OpenAL + glad** | Splits window/input from audio; GLFW?fs Emscripten story is weaker than SDL2. |
| **SDL3** | Fine later. SDL2 is the documented Emscripten + GLES3 pairing today. |
| **SDL2 renderer / GPU API** | 2D abstraction; cannot express this FFP/mesh path. |
| **Native Metal / Vulkan, web WebGL separately** | Two FFP stacks. Rejected. |
| **Desktop GL 2.1 compatibility / `LEGACY_GL_EMULATION`** | Looks like FFP, dies on core/web, and invites ?gjust use glBegin.?h |
| **ANGLE everywhere** | Valid macOS GLES3-over-Metal option if Apple GL 4.1 becomes a blocker. Not the first implementation; revisit if 4.1 missing features appear. |
| **OpenAL replaced by SDL2 audio** | `CWave::SetPos` / listener are 3D. SDL2 audio is a mixer, not a 3D scene. Emscripten already exposes OpenAL. |

## Consequences

### We will

- Create the SDL2 window and GLES/GL core context in the portable `lib/window` / `lib/graphic` bring-up (M3 #8), not in game code.
- Keep `#include <d3d8.h>` in `headers.h`. The GL backend lives behind `IDirect3DDevice8` and D3DX entry points.
- Upload `DrawPrimitiveUP` through a ring of dynamic VBOs (no client arrays), so native core and WebGL2 share the same path.
- Author shaders as **GLSL ES 3.00** plus a desktop preamble (`#version 330 core` / `410 core`, strip `precision`). One semantic source.
- Link native with SDL2 + OpenAL Soft via CMake (`find_package` / pkg-config). Emscripten: `-sUSE_SDL=2`, WebGL2 flags, Emscripten OpenAL. No WASM work in M1.
- Treat stencil shadows as in-scope for FFP completeness (#5), not as a visual gold standard.

### We will not

- Replace `D3DCOLOR` / `VEC3` / `MTX4` with glm types in game or in `lib/` public headers.
- Depend on D3D8 SDK, MinGW, or a Windows-only GL layer for CI.
- Ship communication-wire compatibility (#11) or Capture/AVI (#17) as part of this backend.
- Enable Emscripten legacy FFP emulation to ?gget something on screen.?h

## CMake / CI notes (for #3)

M1 CMake should grow a **linkable native target** that can later `find_package(SDL2)` without requiring SDL at the current object-only `check` preset. SDL/OpenAL as hard dependencies belong when the first window TU is allowlisted (M3), not as a gate that turns M0 `check.sh` red.

Optional preset `native` (link SDL2) vs `check` (stubs only) is the expected split.

## Related

- ABI freeze: `docs/porting/api-surface.md` (issue #1 / PR #20)
- Dev env: [`dev-env.md`](dev-env.md)
- Emscripten GL modes: https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html
- SDL Emscripten: https://github.com/libsdl-org/SDL/blob/SDL2/docs/README-emscripten.md
