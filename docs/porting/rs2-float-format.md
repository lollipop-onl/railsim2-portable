# Save-side float format (`%f`)

- **Issue**: [#50](https://github.com/lollipop-onl/railsim2-portable/issues/50) (parent [#10](https://github.com/lollipop-onl/railsim2-portable/issues/10))
- **Header**: `port/rs2_float.h` (`RS2_FLOAT_FMT`, `rs2_format_float`, `rs2_parse_float`)
- **ctest**: `rs2_float_self_test` (`port/rs2_float_test.cpp`)

## Contract

Legacy RailSim2 Save uses C `fprintf` with `%f`. On 32-bit Windows that prints **six digits after the decimal point** (for example `139.665771`, `-0.000000`). Portable Save must keep that text shape so Load (`ConstValue` / `ConstFloat` in `Script.cpp`, which uses `sscanf(..., "%f", ...)`) round-trips.

`RS2_FLOAT_FMT` is `"%.6f"`. Use it anywhere Save writes a layout float (including `V3Save` in `SystemCover.h`).

Version fields (`RailSimVersion = %.2f`) and UI/debug `FlashIn` / `sprintf` paths are **not** part of this contract.

## Write inventory (Save / plugin Save)

| Location | Pattern |
|----------|---------|
| `SystemCover.h` | `V3Save` ? vector `(x, y, z)` triples |
| `CCamera.cpp` | `Head`, `Pitch`, `Dist`, `FieldOfView` + `V3Save` focus |
| `CWindowDivInfo.cpp` | `HorzRatio`, `VertRatio` |
| `CTrainGroup.cpp` | `TargetSpeed`, `CurrentSpeed`, `StopTarget` + `V3Save` cabin |
| `CTrain.cpp` | `V3Save` old position / tilt |
| `CTrainSetCurve.cpp` | `Location` offset |
| `CRailWay.cpp` | `DummyTrackInterval`, `PierPos`, `PolePos` |
| `CRailConnector.cpp` | `PierLink`, `PoleLink`, `Cant` + `V3Save` pose |
| `CRailwayMode.cpp`, `CRailwayPluginSet.cpp` | `TrackInterval` |
| `CProfilePlugin.cpp` | height map comma list |
| `CPier.cpp` | `SurfaceAlt` + joint `V3Save` |
| `CParticle.cpp` | `ParticleState` credit + `V3Save` |
| `CModelInst.cpp`, `CCustomizerMover.cpp` | `V3Save` pose |
| `CLine.cpp` | `V3Save` line geometry |
| `CDiaInst.cpp` | dia `Offset` |
| `CSaveFile.cpp` | wind `V3Save` |

## Read inventory (Load)

`ConstFloat` / `ConstValue` in `Script.cpp` parse floats for vectors, plugin fields, and curve offsets. Save write sites above must stay compatible with that parser.

## Tests

`rs2_float_self_test` locks `RS2_FLOAT_FMT` output against literals from `Distribution/en/RailSim2/Layout/Sample.rs2` and checks format Å® `strtof` Å® bitwise float equality. Full `Sample.rs2` byte identity remains parent #10 / `rs2_roundtrip`.
