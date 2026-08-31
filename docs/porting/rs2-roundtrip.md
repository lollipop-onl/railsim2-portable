# Sample.rs2 load-save roundtrip harness

- **Issue**: [#24](https://github.com/lollipop-onl/railsim2-portable/issues/24) (parent [#10](https://github.com/lollipop-onl/railsim2-portable/issues/10))
- **Binary**: `rs2_roundtrip` (`port/rs2_roundtrip.cpp`), built by the `check` preset
- **ctest**: `rs2_roundtrip` and `rs2_roundtrip_reports_diff`

This is the mechanical gate for `#10`'s "load then save, byte diff zero" goal. `#36` wires `rs2_layout_roundtrip()` through `CSaveFile::Load` + `CSaveFile::Save` (path-seams option 1: `g_BaseDir` + Layout basenames). Object-graph `Read`/`Save` are still stubbed, so the ctest is marked `WILL_FAIL` until byte-identity work in `#10`. Do not "fix" `%p` width, MD5, or float format in the harness.

## Fixture

Default input is the in-repo English sample:

`Distribution/en/RailSim2/Layout/Sample.rs2`

No extra copy is vendored under `tests/` or into CI. GitHub Actions already checks out `Distribution/`. If that file is missing (sparse checkout, deleted tree), `rs2_roundtrip` prints `skip: no fixture at ...` and exits **77**. CMake sets `SKIP_RETURN_CODE 77`, so `./scripts/check.sh` stays green.

Japanese `Distribution/jp/RailSim2/Layout/Sample.rs2` is CP932; pass it locally if you want that encoding in the compare.

## Local

```bash
./scripts/check.sh
# or, after cmake --build --preset check:
ctest --preset check --output-on-failure -R rs2_roundtrip

# explicit paths (any .rs2):
./build/check/rs2_roundtrip Distribution/en/RailSim2/Layout/Sample.rs2 Distribution/en/RailSim2/Layout/rs2_roundtrip_out.rs2
```

`rs2_roundtrip_reports_diff` feeds two different buffers to the comparator and checks that the failure text is `roundtrip diff`, not a missing-harness error.

## After `#36` (current)

`rs2_roundtrip` links `CSaveFile.cpp` plus `port/rs2_roundtrip_stubs.cpp`. A real byte-identical save still needs the object graph and `%p` / MD5 / float work under `#10`. Until then the test is expected to exit 1 with **roundtrip diff** (ctest `WILL_FAIL`); missing fixture still exits 77 and skips.
