# Mac / CI development environment

M0 goal: one non-interactive command (`./scripts/check.sh`) that matches CI.

## Prerequisites

Install tools with mise (recommended):

```bash
mise install          # reads .mise.toml (cmake, ninja, ccache)
./scripts/check.sh    # auto-adds mise tool paths when needed
```

Legacy fallback (Homebrew):

```bash
brew bundle --file Brewfile
```

| Tool | Purpose |
|------|---------|
| cmake | Configure presets |
| ninja | Native builds (objects + linked `railsim2` stub) |
| ccache | Optional compile cache (wired later) |

## Commands

| Command | What it does |
|---------|----------------|
| `./scripts/check.sh` | Gate: encoding guard -> configure -> build (link) -> ctest -> progress JSON |
| `./scripts/encoding-guard.sh` | CP932 / BOM / SJIS-0x5C literal checks |
| `./scripts/progress.sh` | Prints `N/254` JSON from `port/native_sources.txt` |
| `cmake --preset check` | Configure AppleClang native target |
| `cmake --build --preset check` | Compile allowlisted sources and link `railsim2` |
| `ctest --preset check` | Smoke + `Sample.rs2` roundtrip + text `.x` load + path join (see [rs2-roundtrip.md](rs2-roundtrip.md), [x-file-parser.md](x-file-parser.md), [path-seams.md](path-seams.md)) |

## Compile firewall

Windows/DirectX headers are stubbed under `port/stub/` and injected with `-isystem port/stub` **before** system includes. Game code keeps `#include <d3d8.h>` etc.; only the include path changes.

Native targets are listed in `port/native_sources.txt`. CMake compiles those into `railsim2_native.a` and links a stub `railsim2` from `port/native_entry.cpp` (`main` returns 0). Game objects are **not** linked into the executable yet; they still need udx globals from `lib/`. SDL2 is not required for the `check` preset.

Progress denominator **254** = root-level `*.cpp` + `*.h` game files. Adding a line to the allowlist is monotonic progress.

`stdafx.h` already parses through the stubs. Remaining root `.cpp` files fail mainly on **game header include-order** (`CTrain`, `CDragInterface`) and MSVC-only extra qualification -- not on missing D3D types. Do not rewrite those headers in M1.

Win32 `.rc` is skipped on native. Icons stay as files until a later loader. Sources stay CP932 (M0 encoding-guard). `stdafx.h` already uses `lib/udx.h` with forward slashes.

## Compiler choice

Primary: **AppleClang** (`check` preset).

- Game root string literals are ASCII; UI text comes from `Language.txt`.
- CP932 comments are allowed via `-Wno-invalid-source-encoding`.
- Two debug strings that contained SJIS trail byte `0x5C` were reworded to ASCII in `lib/debug.cpp` and `lib/graphic.cpp`.

Fallback: Homebrew GCC with CP932 input charset (`check-gcc` preset). Use when AppleClang reports source-encoding issues:

```bash
brew install gcc   # uncomment in Brewfile first
cmake --preset check-gcc
cmake --build --preset check-gcc
ctest --preset check-gcc --output-on-failure
```

## MinGW cross-compile (not supported)

We do **not** vendor DirectX SDK headers or adopt MinGW for CI. The compile-firewall path gives early compile signal without license or SDK packaging issues. Manual MinGW experiments are out of scope for M0.

## CI

GitHub Actions runs `./scripts/check.sh` on a **macOS + Linux matrix** (`macos-15`, `ubuntu-24.04`). Both jobs use `mise install` for cmake/ninja; Linux also installs `clang` from apt. Local Mac dev can use the same path via `.mise.toml`, or legacy `brew bundle`.

## Next milestones

- **#10** replaces the roundtrip passthrough with `CSaveFile` and drives byte-identity (`%p` / MD5 / float). The ctest harness itself is [#24](https://github.com/lollipop-onl/railsim2-portable/issues/24) ([rs2-roundtrip.md](rs2-roundtrip.md)).
- **#6** replaces `CXFile` / `D3DXLoadMeshFromX` with the closed parser in `port/xfile.cpp`. Loading Distribution `.x` into memory is [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) ([x-file-parser.md](x-file-parser.md)).
- Backend / window bring-up uses SDL2 only when a `native` preset is added (see `adr-backend.md`); keep `check` stub-only.
