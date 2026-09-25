# D3DX8 math conventions

- **Issue**: [#204](https://github.com/lollipop-onl/railsim2-portable/issues/204) (parent [#5](https://github.com/lollipop-onl/railsim2-portable/issues/5))
- **Declarations**: `port/stub/d3dx8.h`; **definitions**: `port/d3dx_math.cpp`
- **ctest**: `rs2_d3dx_math_self_test` (`port/d3dx_math_test.cpp`)

## Conventions

The game was written against D3DX8, so every function here keeps D3DX8's conventions.

- **Left-handed.** +x right, +y up, +z into the screen. `LookAtLH` puts the target on +z.
- **Row vectors.** A point transforms as `v * M`, so `A * B` applies A first. `_41 _42 _43` is the translation row. `YawPitchRoll(y, p, r)` is `RotZ(r) * RotX(p) * RotY(y)`.
- **Rotation sense.** A positive angle turns +y into +z (`RotationX`), +z into +x (`RotationY`) and +x into +y (`RotationZ`). `RotationQuaternion` of `(sin(a/2) * axis, cos(a/2))` equals `RotationAxis(axis, a)`.
- **Depth range [0, 1].** `PerspectiveFovLH` / `PerspectiveOffCenterLH` map the near plane to z = 0 and the far plane to z = 1, with w = view-space z.
- **Planes.** `(a, b, c, d)` with `a*x + b*y + c*z + d`. `PlaneFromPoints` uses `normalize(cross(v2 - v1, v3 - v1))`; `BoxTest` (`lib/object.cpp`) depends on that winding for its six faces.
- **Shadow.** `MatrixShadow` normalizes the plane, then builds `dot(P, L) * I - P (x) L` as native d3dx does. MSDN prints the negation of that matrix; it lands on the same points with w < 0, so every shadow would be clipped.
- **FVF sizes.** Diffuse / specular count 4 bytes (the Win32 `D3DCOLOR`), not `sizeof(DWORD)`, which is 8 on LP64 hosts.
- **Degenerate input** follows D3DX8, not a safer guess: `Vec3TransformCoord` divides by w even when it is 0, and `MatrixInverse` returns NULL for a singular matrix and leaves the output untouched.

Results match D3DX8 to float rounding, not bit for bit: native D3DX8 picks CPU-specific code paths, so no single bit pattern is "the" answer. The test compares within 1e-5 (1e-4 for a product with an inverse).

## Not implemented

Only what the game calls is here. `D3DXComputeBoundingSphere` is commented out at its only call (`lib/mesh.cpp:442`), and `D3DXVec3Project` / `D3DXVec3Unproject` have no caller, so they stay absent or stubbed until one appears.
