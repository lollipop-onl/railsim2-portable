# MD5 and layout digest

- **Issue**: [#44](https://github.com/lollipop-onl/railsim2-portable/issues/44) (parent [#10](https://github.com/lollipop-onl/railsim2-portable/issues/10))
- **Sources**: `md5.cpp`, `md5.h`, `Network.cpp` (`CheckLayoutDigest`), `CSaveFile.cpp` / `CFileMode.cpp` (callers)
- **ctest**: `rs2_md5_self_test` (`port/md5_digest_test.cpp`)

## Contract

`MD5` is the vendored RSA-derived implementation in `md5.cpp`. Callers:

1. `update` bytes (layout text buffer)
2. `finalize`
3. compare `raw_digest()` (16 bytes) with the session expected digest

`CheckLayoutDigest` (`Network.cpp`) compares against `g_NetworkFileDigest[16]`. On mismatch it shows `LayoutDataHashMismatch` and returns `false`. Network play sets the expected digest; offline load uses the check when `checkhash` is enabled in `CSaveFile::Load`.

`rs2_roundtrip` stubs `CheckLayoutDigest` as a no-op so incomplete object graphs can still exercise Load/Save plumbing (#36).

## Tests

`rs2_md5_self_test` checks RFC-style empty string digest and **locks vendored `md5.cpp` output** for `"abc"` (hex `900150983cd24fb0d6963f7d28e17f72` on this tree). Full `Sample.rs2` byte identity remains parent #10 / `rs2_roundtrip`.
