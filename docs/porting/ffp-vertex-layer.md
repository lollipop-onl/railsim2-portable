# FVF vertex layer (`port/ffp_fvf`)

- **Issue**: [#74](https://github.com/lollipop-onl/railsim2-portable/issues/74) (parent [#5](https://github.com/lollipop-onl/railsim2-portable/issues/5))
- **Depends on**: [ffp-render-states.md](ffp-render-states.md) closed FVF set
- **Related**: [adr-backend.md](adr-backend.md) (shader-per-FVF; `DrawPrimitiveUP` Å® dynamic VBO ring)

## What this slice is

Port-layer tables and CPU storage for the **M3-required** FVF aliases (`FVF_TL` Åc `FVF_NX2`). Game `lib/vertex.cpp` stays off the allowlist. There is no GLSL and no FFP state emulation.

| API | Role |
|-----|------|
| `rs2_ffp_layout` / `rs2_ffp_stride` | Closed FVF Å® byte stride + attribute offsets (`position` / `rhw` / `normal` / `diffuse` / `tex0` / `tex1`) |
| `rs2_ffp_vb_*` | CPU vertex buffer; `Lock` / `Unlock` expose a writable span (caller `memcpy`) |
| `rs2_ffp_set_fvf` + `rs2_ffp_draw_primitive_up` | Record one UP draw: vertex bytes + FVF + primitive type + count. **Draw is a no-op.** |

Unknown FVF values, including deferrable `FVF_S` (`D3DFVF_XYZ` only), return `false` / `E_FAIL`. Do not add `FVF_S` here; that is the shadow-volume slice.

D3D8 `CreateVertexBuffer` / `DrawPrimitiveUP` / `IDirect3DVertexBuffer8` in `port/stub/d3d8.h` remain no-ops so allowlisted game TUs do not take a link dependency yet. Wire those methods to this API when a later slice implements the GL device.

## Closed strides (must match `lib/vertex.h` structs)

| Alias | Bits | Stride |
|-------|------|--------|
| `FVF_TL` | `XYZRHW \| DIFFUSE` | 20 |
| `FVF_TLX` | `XYZRHW \| DIFFUSE \| TEX1` | 28 |
| `FVF_L` | `XYZ \| DIFFUSE` | 16 |
| `FVF_LX` | `XYZ \| DIFFUSE \| TEX1` | 24 |
| `FVF_LX2` | `XYZ \| DIFFUSE \| TEX2` | 32 |
| `FVF_N` | `XYZ \| NORMAL \| DIFFUSE` | 28 |
| `FVF_NX` | `XYZ \| NORMAL \| DIFFUSE \| TEX1` | 36 |
| `FVF_NX2` | `XYZ \| NORMAL \| DIFFUSE \| TEX2` | 44 |
| `FVF_S` | `XYZ` | **rejected** |

Decode order is the D3D FVF packing order: position (`xyz` or `xyz+rhw`), optional normal, optional diffuse, then 8-byte UV pairs. Diffuse is always **4-byte `D3DCOLOR`**, not host `sizeof(DWORD)` (8 on LP64). Offsets use `RS2_FFP_ABSENT` when a field is not in the FVF.

`lib/vertex.h` declares the diffuse field of every `VTX_*` as `D3DCOLOR`, and `port/stub/d3d8.h` makes `D3DCOLOR` a `std::uint32_t` ([#203](https://github.com/lollipop-onl/railsim2-portable/issues/203)). Before that the field was `DWORD`, so `sizeof(VTX_TLX)` was 32 on LP64 against a stride of 28 and `rs2_ffp_draw_primitive_up` rejected every game draw. `DWORD` itself stays `unsigned long`: game code keeps pointers in `DWORD` ([#155](https://github.com/lollipop-onl/railsim2-portable/issues/155)), so narrowing it would truncate them.

ctest `rs2_vertex_layout_self_test` (`port/vertex_layout_test.cpp`) includes the real `lib/vertex.h` and pins, per `VTX_*`: `sizeof` equals `rs2_ffp_layout(...).stride`, every field `offsetof` equals the layout offset (absent fields are `RS2_FFP_ABSENT`), and `DrawPrimitiveUP` accepts `sizeof` as the stride. `static_assert`s in the same file pin `sizeof(D3DCOLOR) == 4`, the Win32 strides above, and `FVF_*` == `RS2_FVF_*`.

## Entrance for the GL slice

1. Select a shader variant by `Rs2FfpLayout.fvf` (or `attrs`).
2. Bind a real VBO from `rs2_ffp_vb_*` storage, or upload `Rs2FfpUpRecord` through a dynamic VBO ring ([adr-backend.md](adr-backend.md): no client arrays).
3. `rs2_ffp_up_last` / `rs2_ffp_up_at` are valid until the next wrap (`16` slots) or `rs2_ffp_up_reset`.

ctest: `rs2_ffp_fvf_self_test` (`port/ffp_fvf_test.cpp --self-test`).
