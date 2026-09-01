# `lib/anim.cpp` and `.x` animation dependency inventory

- **Issue**: [#51](https://github.com/lollipop-onl/railsim2-portable/issues/51) (parent [#6](https://github.com/lollipop-onl/railsim2-portable/issues/6))
- **Depends on**: [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) closed mesh set ? [x-file-templates.md](x-file-templates.md), [x-file-parser.md](x-file-parser.md)
- **Scope**: docs only. No parser or `lib/mesh.cpp` changes.

## Executive summary

`CAnim` (`lib/anim.cpp`) **does not call any X-File API**. It reads a **custom text sidecar** with `fopen` / `fscanf`, loads static `.x` meshes through `CMesh::Load`, and animates them with per-frame position / quaternion tables plus D3DX math helpers. No caller of `CAnim::Load` or `CAnim::Render` exists in this tree, and **Distribution ships no sidecar files** in that format.

Shipped animation is elsewhere: **texture UV animation** (`CTextureAnimation` / `DefineAnimation` in plugin profiles) and **mover customizers** (`StaticRotation`, `DynamicRotation`, …). Those paths touch `.x` only as static meshes already covered by the [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) closed set. **No `Distribution/**/*.x` file instantiates `Frame`, `AnimationSet`, `AnimationKey`, or related X animation templates.**

---

## `lib/anim.cpp` ? direct dependencies

| Symbol / API | Role | X-File? |
|--------------|------|---------|
| `fopen` / `fscanf` / `fclose` | Read custom sidecar (mesh count, mesh paths, frame count, pos/quat rows) | No |
| `CMesh::Load(FALSE, path)` | Load each listed mesh from disk | **Indirect** ? see below |
| `D3DXQuaternionSlerp` | Interpolate keyframe quaternions | No (D3DX math stub) |
| `D3DXMatrixRotationQuaternion` | Build rotation matrix from quaternion | No |
| `CObject::SetMesh` / `SetMatrix` / `Render` | Draw interpolated pose | No |

Sidecar layout (from `CAnim::Load`):

1. `int` mesh count
2. For each mesh: path string → `CMesh::Load(FALSE, path)` (typically a `.x` under the plugin cwd)
3. `int` frame count
4. For each frame × mesh: `pos.x, pos.y, pos.z quat.x, quat.y, quat.z, quat.w`

There is no `CXFile`, `DirectXFileCreate`, `D3DXLoadMeshFromX`, or walk of `AnimationSet` / `Frame` hierarchy.

`lib/anim.cpp` is **not** in `port/native_sources.txt` at this commit. `anim.h` is pulled in through `lib/udx.h` for the UDX bundle only.

---

## Related translation units (animation, not X skeletal)

These implement the animation paths Distribution actually uses. None add X-File animation templates beyond static mesh load.

| TU | What it animates | `.x` touch |
|----|------------------|------------|
| `CTextureAnimation.cpp` | Plugin `DefineAnimation` blocks ? texture frames, UV slides/tiles/rotations | Meshes loaded normally; animation swaps `CMesh` material textures / UV transforms |
| `CCustomizerMisc.cpp` (`CAnimationApplier`) | Binds a named `CTextureAnimation` to a material slot via `SetAnimation = matId, "name"` | Same |
| `CCustomizerMover.cpp` | `StaticRotation`, `DynamicRotation`, timing fields (`AnimationTime`, …) on `CMoverState` | None directly |
| `CModelPlugin.cpp` | Parses `DefineAnimation`, loads frame textures in `LoadData` | None |
| `CModelInst.cpp` | Persists `TexAnimState` in layouts / instances | None |
| `lib/mesh.cpp` (`CXFile`, `CMesh::Load`) | **Only** live X-File entry point: `DirectXFileCreate`, `GetTopMesh` → `TID_D3DRMMesh`, `D3DXLoadMeshFromX` / `D3DXLoadMeshFromXof` (or portable `rs2_cmesh_probe_xfile`) | Static mesh subset only |

Plugin grammar `Frame = "texture.png", length;` in `CTexAnimFrame::Read` is a **texture keyframe**, not an X-File `Frame` template.

---

## Distribution census

Re-run from repo root:

```bash
python3 - <<'PY'
from pathlib import Path
from collections import Counter
import re

# X-File animation templates
anim = {"Frame", "FrameTransformMatrix", "AnimationSet", "Animation",
        "AnimationKey", "AnimationOptions", "CompressedAnimationSet",
        "SkinWeights", "XSkinMeshHeader"}
rx = re.compile(r"(?:^|[\n\r;])[ \t]*(?!template\b)([A-Za-z_][A-Za-z0-9_]*)(?:[ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{")
hits = Counter()
for p in Path("Distribution").rglob("*.x"):
    for n in set(rx.findall(p.read_text(errors="replace"))):
        if n in anim:
            hits[n] += 1
print("animation template hits in .x:", dict(hits) or "none")

# Texture animations (jp tree; en is byte-identical pairs)
profiles = list(Path("Distribution/jp/RailSim2").rglob("*2.txt"))
names = set()
frame_kw = Counter()
for p in profiles:
    t = p.read_text(errors="replace")
    for m in re.finditer(r'DefineAnimation\s+"([^"]+)"', t):
        names.add(m.group(1))
    for kw in ("Frame =", "NumberedFrame", "SlideUVFrame", "TiledUVFrame", "RotationUVFrame"):
        if kw.replace(" =", "") in t or kw in t:
            frame_kw[kw.split()[0]] += 1
print("unique DefineAnimation names:", sorted(names))
print("frame keywords in jp *2.txt:", dict(frame_kw))
PY
```

Results at this document's commit:

### `.x` files (178 total)

| Check | Result |
|-------|--------|
| `AnimationSet` / `Animation` / `AnimationKey` | **0 files** |
| `Frame` / `FrameTransformMatrix` (X hierarchy) | **0 files** |
| `SkinWeights` / `XSkinMeshHeader` | **0 files** |
| Instantiated mesh set | Same as [x-file-templates.md](x-file-templates.md): `Header`, `Mesh`, `MeshMaterialList`, `Material`, optional normals/UV/texture/vertex colors |

### Texture animation in plugin profiles

Five unique `DefineAnimation` names (jp; en copies match):

| Name | Plugin | Frame kind |
|------|--------|------------|
| `Blink1`, `Blink2` | `Station/SingleCrossing`, `Station/DoubleCrossing` | `Frame` + `ShiftTexture` |
| `Escalator` | `Station/MM02` | `SlideUVFrame` |
| `Logo` | `Train/Aizentranza01` | `TiledUVFrame` |
| `コロン点滅` (colon blink) | `Struct/DigitalClock` | `Frame` + `ShiftTexture` |

Frame keyword usage in jp `*2.txt`: `Frame` 6, `SlideUVFrame` 1, `TiledUVFrame` 1. `NumberedFrame` and `RotationUVFrame` do not appear in Distribution (supported by code, unused in shipped assets).

Animated plugins reference many `.x` meshes (`Body.x`, `Signal.x`, `Model.x`, …). All are static meshes within the [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) closed set.

### `CAnim` sidecar files

**None.** No Distribution file matches the sidecar shape (leading integer mesh count, bare paths, quaternion rows). Mechanical motion (e.g. `Struct/EngineTest`, turntable / traverser stations) uses plugin **mover customizers** and rigid `.x` parts, not `CAnim`.

---

## Diff vs [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) closed set

| [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) / [x-file-templates.md](x-file-templates.md) | Animation inventory (#51) |
|---------------------------------------------------------------------------------------------------------------------|---------------------------|
| Required: `Mesh`, `MeshMaterialList`, `Material` | Unchanged ? animated assets use the same static meshes |
| Optional: `MeshNormals`, `MeshTextureCoords`, `TextureFilename`, `MeshVertexColors` | Unchanged |
| Explicitly out of scope: `Frame`, `AnimationSet`, `AnimationKey`, skinning | **Confirmed absent** from all 178 `.x` files; `CAnim` does not need them even if sidecars existed |
| Parser rejects binary / compressed X | Irrelevant to animation ? no such assets |

**Net:** extending the X parser for skeletal / `AnimationSet` data is **not** required for Distribution animation or for `CAnim`. The only `.x` dependency on the anim path is the existing static mesh load inside `CMesh::Load`, already bounded by #28.

---

## Porting boundaries (follow-on work, not #51)

1. **Texture animation** ? no X parser change; needs `CTextureAnimation` / customizer port and texture loading (#6 mesh path).
2. **Mover customizers** ? plugin DSL + `CMoverState`; no `.x` animation templates.
3. **`CAnim`** ? optional future slice if a sidecar appears; would reuse #28 mesh load + add a text sidecar reader in `port/` or `lib/`, not X `AnimationSet` parsing.

`./scripts/check.sh` is unchanged (docs only).
