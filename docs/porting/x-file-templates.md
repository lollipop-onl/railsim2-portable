# X-File templates used by Distribution assets

- **Issue**: [#23](https://github.com/lollipop-onl/railsim2-portable/issues/23) (parent [#6](https://github.com/lollipop-onl/railsim2-portable/issues/6))
- **Tree**: `Distribution/**/*.x` at this document's commit
- **Parser**: not implemented here. `#6` should implement **this closed set**, not the full X-File spec.

`CMesh::Load` currently calls `D3DXLoadMeshFromX` / `D3DXLoadMeshFromXof` after `CXFile::GetTopMesh` walks to `TID_D3DRMMesh`. A portable loader only has to feed that same mesh object. `CAnim::Load` reads a **custom text sidecar** (`fopen` / `fscanf`), not `AnimationSet`.

## Format census

178 files. `jp/` and `en/` are **89 byte-identical pairs** (89 unique blobs).

| Magic | Count | Notes |
|-------|------:|-------|
| `xof 0302txt 0064` | 176 | 64-bit float size in the 16-byte header |
| `xof 0302txt 0032` | 2 | `Skin/Default_Blue/Arrow.x` (jp + en copy) |
| `xof Åcbin` / `tzip` / `bzip` | 0 | **do not implement** binary or compressed X |

Every file is DirectX **text** X-File version **0302**. No compressed, no binary.

`Arrow.x` is the only file that **omits the `template` block** (it instantiates standard names against the D3DRM defaults). The loader must not require in-file template declarations.

## Instantiated templates

Counts are files that contain at least one **data object** of that name (not merely a `template Foo {` declaration). jp+en copies both count.

| Template | Files | Role |
|----------|------:|------|
| `Header` | 178 | version/flags blob; skip after parse |
| `Mesh` | 178 | **required.** top-level object `GetTopMesh` already selects |
| `MeshMaterialList` | 178 | nested under `Mesh` |
| `Material` | 178 | nested under `MeshMaterialList` |
| `MeshNormals` | 176 | nested under `Mesh`. Missing on `Skin/Default_Blue/Compass1.x` only ? must be optional |
| `MeshTextureCoords` | 160 | nested under `Mesh`. Absent on some UI / engine-test meshes |
| `TextureFilename` | 72 | nested under `Material`; `CMesh::Load` already reads `pTextureFilename` |
| `MeshVertexColors` | 18 | pier meshes only (`Pier/**`). Keep |

No file instantiates `Frame`, `FrameTransformMatrix`, `AnimationSet`, `Animation`, `AnimationKey`, `SkinWeights`, or `XSkinMeshHeader`.

## Declared but never instantiated

176 files dump the standard D3DRM template library at the top. These names appear only as `template` declarations (and as field types inside those declarations):

`Boolean`, `Boolean2d`, `ColorRGB`, `ColorRGBA`, `Coords2d`, `IndexedColor`, `MaterialWrap`, `Matrix4x4`, `MeshFace`, `MeshFaceWraps`, `Vector`.

A closed parser can treat them as **built-in member types** of `Mesh` / `Material`. It does not need a general template-registration engine.

## Out of scope for `#6`

Safe to leave unimplemented until a new asset proves otherwise:

- Binary (`bin`) and compressed (`tzip` / `bzip`) X-File
- `Frame` hierarchy / `FrameTransformMatrix`
- `AnimationSet` and related animation templates (game animation is `CAnim`, not X)
- Skinning (`SkinWeights`, `XSkinMeshHeader`)
- Patches, effects, `CompressedAnimationSet`, and the rest of the DirectX 9 template pack

M2 only needs an in-memory mesh. Drawing is `#5`.

## Instance-set buckets

| Files | Instantiated set |
|------:|------------------|
| 88 | `Header` `Mesh` `MeshMaterialList` `Material` `MeshNormals` `MeshTextureCoords` |
| 54 | above + `TextureFilename` |
| 18 | above + `TextureFilename` + `MeshVertexColors` |
| 16 | `Header` `Mesh` `MeshMaterialList` `Material` `MeshNormals` (no UVs) |
| 2 | `Header` `Mesh` `MeshMaterialList` `Material` (`Compass1.x`; no normals, no UVs) |

## Verification

Re-run from the repo root if `Distribution/` moves:

```bash
python3 - <<'PY'
from pathlib import Path
from collections import Counter
import re
files = list(Path("Distribution").rglob("*.x"))
magic = Counter(p.read_bytes()[:16].decode("ascii", "replace") for p in files)
print("files", len(files))
print(magic)
inst = Counter()
rx = re.compile(r"(?:^|[\n\r;])[ \t]*(?!template\b)([A-Za-z_][A-Za-z0-9_]*)(?:[ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{")
for p in files:
    names = set(rx.findall(p.read_text(errors="replace")))
    for n in names:
        inst[n] += 1
print(inst)
PY
```

`./scripts/check.sh` is unchanged (docs only).
