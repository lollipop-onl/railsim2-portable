# Internal string representation (ADR)

- **Issue**: [#46](https://github.com/lollipop-onl/railsim2-portable/issues/46) (parent [#9](https://github.com/lollipop-onl/railsim2-portable/issues/9))
- **Status**: accepted for M2+ implementation slices

## Context

- Game sources and `.rs2` layout files use **CP932** on disk (`docs/porting/rs2-roundtrip.md`).
- `lib/headers.h` still pulls Win32 `<mbstring.h>` and `<imm.h>`; game UI uses GDI `HFONT` / `CStringTexture` (`docs/porting/api-surface.md`).
- macOS may expose paths in NFD; Linux paths are case-sensitive (`docs/porting/path-seams.md`).

## Decision

**Adopt UTF-8 as the in-process representation for user-visible text, with CP932 only at file and Win32-compat boundaries.**

| Layer | Encoding |
|-------|----------|
| `.rs2`, plugin text on disk | CP932 (unchanged) |
| `std::string` / UI copy after load | UTF-8 |
| Save / export | UTF-8 -> CP932 at write boundary |
| File names in `Distribution` | Preserve on-disk bytes; normalize only in `port/path*` when required |

Rationale: UTF-8 keeps `std::string` safe on Linux/macOS without widening every call site to `wchar_t`, while round-trip file compat stays explicit at I/O seams.

## Rejected alternatives

1. **CP932 everywhere internally** -- avoids conversion but forces `_mbs*` / SJIS semantics on every host; brittle on Linux libc.
2. **UTF-16 (`wchar_t`) internal** -- matches old Win32 GDI paths but expands scope across `#16` (GDI/font) and does not simplify portable I/O.

## Consequences (follow-up slices, not this ADR)

- Replace `_mbs*` with a small `lib/rs2_text` (or `port/`) facade: `to_utf8(cp932)`, `to_cp932(utf8)`, NFC/NFD only where paths cross OS APIs.
- IME: SDL / backend `TEXTINPUT` events deliver UTF-8; `CEditBox` stops calling `Imm*` directly.
- Tests: Japanese layout name and `Sample.rs2` string fields must **byte-match** Windows saves after load->save (parent #9 / #10).

## Non-goals

- Mass UTF-8 conversion of game sources (`encoding-guard.sh` still applies).
- Changing on-disk CP932 for `.rs2` or plugins.
