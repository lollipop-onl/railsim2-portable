# Closed text X-File parser

- **Issue**: [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) (parent [#6](https://github.com/lollipop-onl/railsim2-portable/issues/6))
- **Set**: [x-file-templates.md](x-file-templates.md)
- **Sources**: `port/xfile.h`, `port/xfile.cpp`
- **Binary**: `rs2_xfile_load` (`port/xfile_load.cpp`), built by the `check` preset
- **ctest**: `rs2_xfile_self_test` and `rs2_xfile_load`

This is the mechanical gate for `#6`'s "`.x` is an in-memory mesh" goal. It does **not** replace `CXFile` or `CMesh::Load` (those still call `D3DXLoadMeshFromX` / `Xof`). `#6` should feed `Rs2XMesh` into that path.

Faces stay **n-gons** as written in the file. Triangulation belongs with the later `ID3DXMESH`-like object, not here.

## Fixture

Default scan root is `Distribution/` (178 files: 89 unique jp/en pairs). GitHub Actions already checks that tree out. If the directory is missing, `rs2_xfile_load` prints `skip: no fixture at ...` and exits **77**.

## Local

```bash
./scripts/check.sh
# or, after cmake --build --preset check:
ctest --preset check --output-on-failure -R rs2_xfile

./build/check/rs2_xfile_load --self-test
./build/check/rs2_xfile_load Distribution
```

`rs2_xfile_self_test` parses an inline Arrow-like mesh and rejects `xof 0302bin`. The Distribution test also checks `Arrow.x` (normals, no UVs) and `Compass1.x` (no normals, no UVs).

## After `#6` wires `CMesh::Load`

Keep this ctest. Wiring should not change the parse result; it only copies `Rs2XMesh` into the existing material / texture lists.
