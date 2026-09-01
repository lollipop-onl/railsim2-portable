# `.x` loader epic (#6) mechanical gate completion inventory

- **Issue**: [#55](https://github.com/lollipop-onl/railsim2-portable/issues/55) (parent [#6](https://github.com/lollipop-onl/railsim2-portable/issues/6))
- **Depends on**: [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28), [#45](https://github.com/lollipop-onl/railsim2-portable/issues/45), [#51](https://github.com/lollipop-onl/railsim2-portable/issues/51) merged
- **Scope**: docs only. No parser, `lib/mesh.cpp`, or render changes.

## Executive summary

Parent [#6](https://github.com/lollipop-onl/railsim2-portable/issues/6) mechanical gates are **complete**: Distribution's 178 text `.x` files parse into `Rs2XMesh`, `CMesh::Load` uses the portable probe under `RS2_PORTABLE_COMPILE_FIREWALL`, and ctests cover the full tree. Template and animation inventories fix the closed parser set; no `Frame` / `AnimationSet` / skinning templates appear in shipped assets.

What remains for [#6](https://github.com/lollipop-onl/railsim2-portable/issues/6) as a whole is **outside this epic's mechanical scope**: mesh draw (M3), texture animation / plugin customizers (M3?M4), and optional upstream `CXFile` removal on Windows. Parser extension for templates Distribution does not use is explicitly **not** required.

---

## Mechanical gates

| Slice | Deliverable | `./scripts/check.sh` gate | Doc |
|-------|-------------|---------------------------|-----|
| [#23](https://github.com/lollipop-onl/railsim2-portable/issues/23) | Template census on `Distribution/**/*.x` | encoding-guard (docs only) | [x-file-templates.md](x-file-templates.md) |
| [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) | Closed text parser `port/xfile.*` Å® `Rs2XMesh` | `rs2_xfile_self_test`, `rs2_xfile_load` (178 files; skip 77 if no `Distribution/`) | [x-file-parser.md](x-file-parser.md) |
| [#45](https://github.com/lollipop-onl/railsim2-portable/issues/45) | `CMesh::Load` file path Å® `rs2_cmesh_probe_xfile` | `rs2_cmesh_xfile_self_test`, `rs2_cmesh_xfile_load` | [x-file-parser.md](x-file-parser.md) |
| [#51](https://github.com/lollipop-onl/railsim2-portable/issues/51) | `lib/anim.cpp` / Distribution animation dependency inventory | encoding-guard (docs only) | [anim-x-inventory.md](anim-x-inventory.md) |
| **#55 (this doc)** | Epic boundary: gates vs follow-on milestones | encoding-guard (docs only) | this file |

All four ctests are registered in `CMakeLists.txt` under the `check` preset. Re-run:

```bash
./scripts/check.sh
# or, after build:
ctest --preset check --output-on-failure -R 'rs2_xfile|rs2_cmesh_xfile'
```

---

## Parent #6 checklist (mechanical vs epic-outside)

| #6 item | Status | Evidence |
|---------|--------|----------|
| Confirm `.x` format (text / binary / compressed) | **done** | 176Å~ `xof 0302txt 0064`, 2Å~ `0032`; 0 bin/tzip/bzip ? [x-file-templates.md](x-file-templates.md) |
| Inventory instantiated templates; bound parser | **done** | Closed set: `Mesh`, `MeshMaterialList`, `Material`, optional normals/UV/texture/vertex colors ? [x-file-templates.md](x-file-templates.md), [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) |
| Replace `CXFile` with own parser in `CMesh::Load` | **done (portable path)** | `rs2_cmesh_probe_xfile` under `RS2_PORTABLE_COMPILE_FIREWALL` ? [#45](https://github.com/lollipop-onl/railsim2-portable/issues/45); upstream Windows branch still calls `DirectXFileCreate` / `D3DXLoadMeshFromXof` |
| Confirm `lib/anim.cpp` animation dependencies | **done** | No X-File animation APIs; static mesh load only ? [anim-x-inventory.md](anim-x-inventory.md), [#51](https://github.com/lollipop-onl/railsim2-portable/issues/51) |
| Load-verify all 178 assets | **done** | `rs2_xfile_load` + `rs2_cmesh_xfile_load` over `Distribution/` |

**Epic-outside (not mechanical gates; do not block closing #6's data path):**

| Work | Milestone | Notes |
|------|-----------|-------|
| Draw parsed mesh (triangulation, FVF buffers, device) | **M3** Bring-up (#5 / #8) | Faces stay n-gons in parser; GPU path is render, not load |
| `CTextureAnimation` / `DefineAnimation` UV animation | **M3?M4** | Five shipped names; no X parser change ? [anim-x-inventory.md](anim-x-inventory.md) |
| Mover customizers (`StaticRotation`, escalators, Åc) | **M4** Playable | Plugin DSL + rigid `.x` parts |
| Remove residual `CXFile` on non-firewall Windows builds | optional cleanup | Portable path already bypasses `d3dxof` |
| `CAnim` custom sidecar reader | optional | No Distribution sidecar files exist |
| `Frame` / `AnimationSet` / skinning parser | **never for shipped assets** | 0 files in 178 `.x` census |

---

## Diff vs [#51](https://github.com/lollipop-onl/railsim2-portable/issues/51) animation inventory

[#51](https://github.com/lollipop-onl/railsim2-portable/issues/51) answered whether parser work must extend beyond the [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) closed set for animation. This epic inventory **confirms and closes** that boundary:

| [#51](https://github.com/lollipop-onl/railsim2-portable/issues/51) finding | #55 epic conclusion |
|------------------------------------------------|---------------------|
| `CAnim` uses a text sidecar, not X `AnimationSet` | No parser slice needed; sidecar format is separate if ever required |
| Shipped motion = `CTextureAnimation` + mover customizers | Follow M3/M4 render and plugin ports, not `port/xfile.*` |
| 0/178 `.x` files instantiate `Frame`, `AnimationSet`, `AnimationKey`, skinning | Parser extension for those templates is **out of scope** for #6 |
| Animated plugins reference static `.x` meshes (`Body.x`, `Signal.x`, Åc) | Already covered by [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) + [#45](https://github.com/lollipop-onl/railsim2-portable/issues/45) gates |

**Net:** [#51](https://github.com/lollipop-onl/railsim2-portable/issues/51) and [#55](https://github.com/lollipop-onl/railsim2-portable/issues/55) agree ? the mechanical `.x` load epic stops at in-memory `Rs2XMesh` + `CMesh::Load` material/texture slots. Nothing in the animation inventory reopens parser scope.

---

## M3 / M4 follow-ons (explicitly not #55)

These depend on the gates above but are **separate issues / milestones**:

### M3 Bring-up

- **Mesh Å® GPU**: triangulate n-gons, build vertex buffers matching legacy FVF, hook `CMesh::Render` / `lib/graphic` device path ([#5](https://github.com/lollipop-onl/railsim2-portable/issues/5), [#8](https://github.com/lollipop-onl/railsim2-portable/issues/8)).
- **Texture load**: `g_TexList.Get` already called from portable `CMesh::Load`; needs real texture backend, not parser work.
- **`CTextureAnimation`**: port `CTextureAnimation.cpp`, `CCustomizerMisc.cpp` applier, plugin `DefineAnimation` parsing ? UV/keyframe animation without X templates.

### M4 Playable

- **Mover customizers**: `CCustomizerMover.cpp` timing / rotation fields on layouts with rigid `.x` parts.
- **End-to-end**: trains, stations, and scenery visible with texture animations (blink, escalator UV slide, logo tile, clock colon).

### Not planned (unless new assets appear)

- Binary / compressed X (`bin`, `tzip`, `bzip`)
- Skeletal / hierarchical X animation templates
- Full `d3dxof` / `CXFile` removal on upstream Windows (nice-to-have after portable path is default)

---

## Related docs

| Doc | Issue |
|-----|-------|
| [x-file-templates.md](x-file-templates.md) | #23 |
| [x-file-parser.md](x-file-parser.md) | #28, #45 |
| [anim-x-inventory.md](anim-x-inventory.md) | #51 |
| [adr-backend.md](adr-backend.md) | #2 (M3 device contract) |
| [api-surface.md](api-surface.md) | M1 freeze; `D3DXLoadMeshFromX` owned by M2 #6 |

`./scripts/check.sh` is unchanged by this document (docs only).
