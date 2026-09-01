# Sample.rs2 load-save roundtrip harness

- **Issue**: [#24](https://github.com/lollipop-onl/railsim2-portable/issues/24) / [#54](https://github.com/lollipop-onl/railsim2-portable/issues/54) (parent [#10](https://github.com/lollipop-onl/railsim2-portable/issues/10))
- **Binary**: `rs2_roundtrip` (`port/rs2_roundtrip.cpp`), built by the `check` preset
- **ctest**: `rs2_roundtrip` and `rs2_roundtrip_reports_diff`

`rs2_layout_roundtrip()` calls `CSaveFile::Load` + `CSaveFile::Save` with `g_BaseDir` + Layout basenames. The harness links real layout object-graph TUs (see `RS2_ROUNDTRIP_SOURCES` in `CMakeLists.txt`) plus `port/rs2_roundtrip_{stubs,seams,plugins,ptr,readmap,float_lexeme}.cpp`.

## Fixture

`Distribution/en/RailSim2/Layout/Sample.rs2`

Missing fixture → exit **77** (`SKIP_RETURN_CODE`).

## Local

```bash
./scripts/check.sh
# or:
cmake --build --preset check --target rs2_roundtrip
ctest --preset check --output-on-failure -R rs2_roundtrip
```

## Diff classification (#54, Sample.rs2)

| Category | Status |
|----------|--------|
| Load hang at `TexAnimState` | **Fixed** — real `CTexAnimState::Read`/`Save` in seams (stub returned non-NULL forever) |
| `%p` width / case | **Fixed** — `rs2_ptr32_serial` reverse-maps `g_AddressMap`; `RS2_PTR_FMT "%08X"` under `RS2_ROUNDTRIP` |
| `DepartureTime` / `PDWORD` on LP64 | **Fixed** — `rs2_asgn_pointer32_pair` on load; `uint32_t[2]` save |
| `Enabled` cleared after load | **Fixed** — 64-bit `AsgnPointer` into `double m_DepartureTime` overwrote adjacent `bool`s |
| `SeekList` / `PointList` order | **Fixed** — `vector` instead of `set` under `RS2_ROUNDTRIP` |
| `DiaInst` map iteration order | **Fixed** — `m_DiaOrder` preserves file order |
| `yes`/`no` literals | **Fixed** — `YESNO[]` uses lowercase in roundtrip stubs |
| Plugin `List()` OOM | **Fixed** — `CPluginList::ListIdsOnly()` + `LoadAndGet` stub |
| Float literals (general) | **Fixed** — `rs2_float_remember` + `RS2_F`/`V3Save` replay lexemes; copy lexemes after `push_back` / pier joint locals |
| Rail height-map vectors | **Fixed** — same lexeme path as scalar floats when values are not reallocated |
| Remaining geometry | **Fixed** — `SplitList`/`CPier` lexeme relocation; `ctest rs2_roundtrip` byte-matches Sample.rs2 |

`ctest rs2_roundtrip` exits **0** on Sample.rs2 (493661 bytes in/out).

## Linked game TUs (roundtrip only)

`CSaveFile`, `Script`, object graph (`CRailWay`, `CTrainGroup`, `CScene`, …), `CPlugin` (with `RS2_ROUNDTRIP` stubs). Plugin type bodies live in `port/rs2_roundtrip_plugins.cpp`. `CPlugin.cpp` is **not** on the `railsim2_native` allowlist (tree UI headers fail the compile firewall).

## `RS2_ROUNDTRIP` Save guards (`CSaveFile.cpp`)

- Skip post-load simulation (`Enter`, `RestoreSet`, `Simulate`)
- Preserve loaded `m_Version`, `m_FileDate`
- Omit empty `RailBlockList`; conditional `Simulation{}` / `Window{}` blocks
- Radio state via `GetCheck()` loops instead of `GetNumber()` on zeroed stub widgets
