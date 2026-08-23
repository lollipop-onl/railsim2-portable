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
| ninja | Object-only builds |
| ccache | Optional compile cache (wired later) |

## Commands

| Command | What it does |
|---------|----------------|
| `./scripts/check.sh` | Full M0 gate: encoding guard Å® configure Å® build Å® ctest Å® progress JSON |
| `./scripts/encoding-guard.sh` | CP932 / BOM / SJIS-0x5C literal checks |
| `./scripts/progress.sh` | Prints `N/254` JSON from `port/native_sources.txt` |
| `cmake --preset check` | Configure AppleClang object library |
| `cmake --build --preset check` | Compile allowlisted sources |
| `ctest --preset check` | Run ctest skeleton (extended by #10) |

## Compile firewall

Windows/DirectX headers are stubbed under `port/stub/` and injected with `-isystem port/stub` **before** system includes. Game code keeps `#include <d3d8.h>` etc.; only the include path changes.

Native targets are listed in `port/native_sources.txt`. CMake reads this allowlist and builds an `OBJECT` library only ?? no link step in M0.

Progress denominator **254** = root-level `*.cpp` + `*.h` game files. Adding a line to the allowlist is monotonic progress.

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

- **#3** grows this CMake skeleton into a linked native binary.
- **#10** attaches `.rs2` roundtrip tests to the ctest harness created here.
