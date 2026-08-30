# Sample.rs2 load-save roundtrip harness

- **Issue**: [#24](https://github.com/lollipop-onl/railsim2-portable/issues/24) (parent [#10](https://github.com/lollipop-onl/railsim2-portable/issues/10))
- **Binary**: `rs2_roundtrip` (`port/rs2_roundtrip.cpp`), built by the `check` preset
- **ctest**: `rs2_roundtrip` and `rs2_roundtrip_reports_diff`

This is the mechanical gate for `#10`'s "load then save, byte diff zero" goal. It does **not** call `CSaveFile` yet (that still needs udx globals). `#10` should replace `rs2_layout_roundtrip()` with `CSaveFile::Load` + `CSaveFile::Save`. Do not "fix" `%p` width, MD5, or float format in the harness.

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
./build/check/rs2_roundtrip Distribution/en/RailSim2/Layout/Sample.rs2 /tmp/out.rs2
```

`rs2_roundtrip_reports_diff` feeds two different buffers to the comparator and checks that the failure text is `roundtrip diff`, not a missing-harness error.

## After `#10` wires `CSaveFile`

A real save will likely differ (`%p` width on 64-bit, `LayoutInfo.Date` from `GetLocalTime`, float `%f`). That must fail as **roundtrip diff** with sizes and the first mismatch offset. Byte-identity is `#10`'s job, not this harness.
