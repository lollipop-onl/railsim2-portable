# Save compat epic (#10) mechanical gate completion inventory

- **Issue**: [#59](https://github.com/lollipop-onl/railsim2-portable/issues/59) (parent [#10](https://github.com/lollipop-onl/railsim2-portable/issues/10))
- **Depends on**: [#40](https://github.com/lollipop-onl/railsim2-portable/issues/40), [#44](https://github.com/lollipop-onl/railsim2-portable/issues/44), [#50](https://github.com/lollipop-onl/railsim2-portable/issues/50), [#54](https://github.com/lollipop-onl/railsim2-portable/issues/54) merged
- **Scope**: docs only. No layout `.rs2` changes, `.x` / render / mesh work, or Config / plugin verification (#58).

## Executive summary

Parent [#10](https://github.com/lollipop-onl/railsim2-portable/issues/10) **layout `.rs2` mechanical gates are complete**: pointer IDs write as 8-digit hex, vendored MD5 matches known vectors, Save-side floats use `RS2_FLOAT_FMT` (`"%.6f"`), and `Sample.rs2` loadÅ®save is byte-identical under `rs2_roundtrip`. Each gate has a dedicated doc and a ctest registered in `CMakeLists.txt` under the `check` preset.

What remains for [#10](https://github.com/lollipop-onl/railsim2-portable/issues/10) as a whole is **outside this inventory's mechanical scope**: Config / plugin definition load compat ([#58](https://github.com/lollipop-onl/railsim2-portable/issues/58)). That slice covers `Config.txt`, `Language.txt`, and representative `*2.txt` plugin profiles ? not layout object graphs or `CSaveFile` Save format.

---

## Mechanical gates

| Slice | Deliverable | `./scripts/check.sh` gate | Doc |
|-------|-------------|---------------------------|-----|
| [#40](https://github.com/lollipop-onl/railsim2-portable/issues/40) | Pointer ID write: 8-digit lowercase hex; `HexPointer` accepts 8- and 16-digit input | `rs2_ptr_self_test` | `port/rs2_ptr.h`, `port/rs2_ptr_test.cpp` |
| [#44](https://github.com/lollipop-onl/railsim2-portable/issues/44) | Vendored `md5.cpp` + `CheckLayoutDigest` contract | `rs2_md5_self_test` | [md5-layout-digest.md](md5-layout-digest.md) |
| [#50](https://github.com/lollipop-onl/railsim2-portable/issues/50) | Save-side float `"%.6f"` write / `ConstValue` parse round-trip | `rs2_float_self_test` | [rs2-float-format.md](rs2-float-format.md) |
| [#54](https://github.com/lollipop-onl/railsim2-portable/issues/54) | `Sample.rs2` loadÅ®save byte identity | `rs2_roundtrip`, `rs2_roundtrip_reports_diff` | [rs2-roundtrip.md](rs2-roundtrip.md) |
| **#59 (this doc)** | Epic boundary: layout gates vs #58 | encoding-guard (docs only) | this file |

Re-run layout gates:

```bash
./scripts/check.sh
# or, after build:
ctest --preset check --output-on-failure -R 'rs2_ptr|rs2_md5|rs2_float|rs2_roundtrip'
```

Fixture note: `rs2_roundtrip` skips with exit **77** when `Distribution/en/RailSim2/Layout/Sample.rs2` is absent (`SKIP_RETURN_CODE`).

---

## Parent #10 checklist (mechanical vs epic-outside)

| #10 item | Status | Evidence |
|---------|--------|----------|
| `%p` output width fixed; 32-bit interop (#40) | **done** | `rs2_ptr_self_test`; `RS2_PTR_FMT` / `rs2_format_ptr` in `port/rs2_ptr.h` |
| `HexPointer` accepts 16-digit input (#40) | **done** | `rs2_ptr_test.cpp` `parse_like_hex_pointer` cases |
| MD5 / `CheckLayoutDigest` match after port (#44) | **done** | `rs2_md5_self_test`; [md5-layout-digest.md](md5-layout-digest.md) |
| Float `%f` digits / rounding (#50) | **done** | `rs2_float_self_test`; [rs2-float-format.md](rs2-float-format.md) |
| `Sample.rs2` readÅ®save diff zero (#54) | **done** | `ctest rs2_roundtrip` pass (493661 bytes in/out); [rs2-roundtrip.md](rs2-roundtrip.md) |
| Config / plugin definition files verified | **open** | [#58](https://github.com/lollipop-onl/railsim2-portable/issues/58) ? see boundary below |

**Epic-outside (not layout mechanical gates; do not block closing #10's `.rs2` path):**

| Work | Issue | Notes |
|------|-------|-------|
| `CConfigMode::Load` / `Save`, `Language.txt` smoke | **#58** | cwd rules in [path-seams.md](path-seams.md) |
| Representative `*2.txt` plugin profile `Load` smoke | **#58** | e.g. `Rail/Rail01/Rail2.txt`; not full plugin tree |
| Windows?portable cross-build byte compare on real saves | follow-on | Gates lock format; field cross-check is manual / CI optional |
| M3 draw, `.x` mesh load, network wire compat | other milestones | Out of #10 save-compat scope |

---

## Diff vs [#58](https://github.com/lollipop-onl/railsim2-portable/issues/58) Config / plugin compat

[#58](https://github.com/lollipop-onl/railsim2-portable/issues/58) answers whether **settings and plugin definition text** loads on the portable build. This epic inventory **confirms and closes** the layout side:

| Layout gate (#40 / #44 / #50 / #54) | #58 scope |
|-------------------------------------|-----------|
| `CSaveFile::Load` / `Save` on `Layout/*.rs2` | `CConfigMode`, `CPlugin::LoadData`, `*2.txt` grammar |
| Pointer / float / MD5 / roundtrip ctests above | Config roundtrip + at least one `*2.txt` smoke ctest (TBD in #58) |
| `port/rs2_roundtrip_*` harness + `RS2_ROUNDTRIP` stubs | No changes to layout Save format or `Sample.rs2` fixture |
| `CPlugin.cpp` linked only in roundtrip harness | Plugin **definition file** I/O, not layout plugin instance graph |

**Net:** [#59](https://github.com/lollipop-onl/railsim2-portable/issues/59) and [#58](https://github.com/lollipop-onl/railsim2-portable/issues/58) are sequential, not overlapping. Do not extend `rs2_roundtrip` or layout TUs for Config / plugin work; add separate ctests and a short doc addendum under `docs/porting/` in the #58 PR (not `config-plugin-compat.md` naming ? that file is #58's deliverable).

---

## Gate detail (ctest names)

All tests below are registered in root `CMakeLists.txt` for the `check` preset.

### `#40` ? pointer IDs (`rs2_ptr_self_test`)

- **Binary**: `rs2_ptr_test` (`port/rs2_ptr_test.cpp`)
- **Contract**: Save writes 8 lowercase hex digits (`0195dca8`, `00000000` for null). Parse accepts 1?16 hex digits (legacy 8-digit and LP64 16-digit Windows `%p` dumps).
- **Sources**: `port/rs2_ptr.h`; Save sites use `RS2_PTR_FMT` / `rs2_format_ptr`. `Script.cpp` `HexPointer` reads layout pointer fields.

### `#44` ? MD5 (`rs2_md5_self_test`)

- **Binary**: `rs2_md5_digest_test` (`port/md5_digest_test.cpp` + `md5.cpp`)
- **Contract**: RFC empty-string digest; locks `"abc"` Å® `900150983cd24fb0d6963f7d28e17f72`. `CheckLayoutDigest` compares 16-byte raw digest when network session enables hash check.
- **Roundtrip note**: `rs2_roundtrip` stubs `CheckLayoutDigest` no-op so Load/Save plumbing can run offline.

### `#50` ? float format (`rs2_float_self_test`)

- **Binary**: `rs2_float_test` (`port/rs2_float_test.cpp`)
- **Contract**: `RS2_FLOAT_FMT` is `"%.6f"`. Self-test literals taken from `Sample.rs2`. Full byte identity delegated to `rs2_roundtrip`.

### `#54` ? Sample.rs2 roundtrip (`rs2_roundtrip`, `rs2_roundtrip_reports_diff`)

- **Binary**: `rs2_roundtrip` (`port/rs2_roundtrip.cpp` + layout object-graph TUs in `RS2_ROUNDTRIP_SOURCES`)
- **Fixture**: `Distribution/en/RailSim2/Layout/Sample.rs2`
- **Contract**: `CSaveFile::Load` then `Save` produces a byte-identical file. `rs2_roundtrip_reports_diff` exercises diff classification without requiring the fixture.

---

## Related docs

| Doc | Issue |
|-----|-------|
| [md5-layout-digest.md](md5-layout-digest.md) | #44 |
| [rs2-float-format.md](rs2-float-format.md) | #50 |
| [rs2-roundtrip.md](rs2-roundtrip.md) | #24, #36, #54 |
| [path-seams.md](path-seams.md) | #30; Config / plugin cwd context for #58 |
| [upstream.md](upstream.md) | port strategy |

`./scripts/check.sh` is unchanged by this document (docs only).
