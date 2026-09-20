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
| `cmake --preset runtime` | Same sources, plus **optional** SDL2 / OpenGL / OpenAL search. Not CI. |

## Compile firewall

Windows/DirectX headers are stubbed under `port/stub/` and injected with `-isystem port/stub` **before** system includes. Game code keeps `#include <d3d8.h>` etc.; only the include path changes.

Native targets are listed in `port/native_sources.txt`. CMake compiles those into `railsim2_native.a` and links a stub `railsim2` from `port/native_entry.cpp` (`main` returns 0). Game objects are **not** linked into the executable yet; they still need udx globals from `lib/`. SDL2 is not required for the `check` preset.

Progress denominator **254** = root-level `*.cpp` + `*.h` game files. Adding a line to the allowlist is monotonic progress.

`stdafx.h` already parses through the stubs. Include-order and MSVC-only extra qualification are no longer what blocks the allowlist. Remaining TUs split into four groups:

| Blocker | Examples |
|---------|----------|
| Missing D3D8 / DirectX header or type in `port/stub/` | `lib/height_field.cpp` / `lib/texture.cpp` (`IDirect3DTexture8::GetLevelDesc`), `lib/mesh.cpp` (no `rmxfguid.h` stub -- fails on both hosts) |
| Missing GDI / Win32 UI types | `CPixelbit.cpp` (`BITMAPFILEHEADER`, `ReadFile`), `lib/font.cpp` (`LOGFONT`, `DT_*`), `lib/texture.cpp`, `lib/debug.cpp` (`OSVERSIONINFO`), `lib/sprite.cpp` (`::SetRect` not in stub) |
| Needs a real backend, not a stub | `lib/comm.cpp` (DirectPlay8), `lib/music.cpp` (DirectMusic), `lib/sound.cpp` / `lib/wave_stream.cpp` (DirectSound) |
| Type mismatch, nothing missing | `lib/height_field.cpp` (`min` / `max` over mixed `int` and `float` -- the game assumes MSVC's `windef.h` macros, which `NOMINMAX` removes), `lib/draw.cpp` (initializer-list narrowing), `CWaveArray.cpp` (MSVC array-new bound expression) |

A TU can sit in more than one row, so the rows are not a partition and the table
alone does not tell you what a stub addition buys. `lib/height_field.cpp` appears
under both a missing type and a type mismatch; `D3DLOCKED_RECT` is now in the
stub and `GetLevelDesc` would be the next addition, yet the TU still fails on
`min` / `max` (3 errors, of which 2 survive `GetLevelDesc`). `lib/texture.cpp`
wants `GetLevelDesc` too, and 18 of its 21 errors sit under the GDI gap.
Before allowlisting a group, compile its TUs with
`-ferror-limit=0` against the flags in `build/check/compile_commands.json` and
check that the list of errors goes to zero, not just that the first one
disappears. Count with `grep -E 'error:'`: `': error:'` misses `fatal error:`,
and CP932 sources need `grep -a` or they are skipped as binary.

### Both hosts, always

CI (`.github/workflows/check.yml`) is a matrix of `macos-15` **and** `ubuntu-24.04`. A TU that compiles on the host you happen to be sitting at is not allowlistable; it has to pass on both.

The rule is not theoretical. Two host asymmetries have been found and fixed, and they pointed in **opposite** directions -- one was fatal only on Linux, the other only on macOS. Either one, measured on the wrong host alone, would have been recorded as a pass. Both are resolved; the traps below outlive them.

#### Include path case -- was fatal on Linux only

`CGameMode.cpp` and `RailSim2.cpp` spelled the include `"RSPV.h"` while the tracked file was `RSPV.H` -- the only `.H` in a tree of 130-plus `.h` headers. macOS resolved it on its case-insensitive filesystem and emitted only `-Wnonportable-include-path`; Linux gave `fatal error: 'RSPV.h' file not found`. The file is now tracked as `RSPV.h` and both TUs are allowlisted.

Two things outlive the fix:

- The `check` preset compiles `railsim2_native` with `-Werror=nonportable-include-path`, so a new case mismatch fails on macOS too instead of waiting for Linux CI. A sweep of every game TU (124 root `*.cpp` plus 28 under `lib/`) is clean under it, allowlisted or not.
- A Docker **bind mount** from a macOS host leaks that case-insensitivity into the container, so a mismatch passes there and the verification proves nothing. Unpack `git archive HEAD` inside the container (overlayfs) and confirm with `ls RSPV.h` / `ls RSPV.H` that the container filesystem really is case-sensitive before measuring.

#### Comparison-operator constness -- was fatal on macOS only

`CLensFlare.cpp` and `CRailwayPluginSet.cpp` called `std::list::sort` on element types whose `operator<` was not `const`-qualified. libc++ invokes the comparator through `const` references, so the operator dropped out of overload resolution (`no matching function for call to 'std::__less<void, void>'`); libstdc++ and MSVC both accepted it. Adding `const` to the two declarations fixed it, and both TUs are allowlisted.

The trigger is the standard library, not the operating system. Linux clang with `libc++-dev` installed and `-stdlib=libc++` reproduces it exactly (`invalid operands to binary expression ('const CFlareElement' and 'const CFlareElement')`); macOS was the only failing leg because it is the only leg that builds against libc++ by default.

What outlives the fix: every `operator<` in the game headers is now `const`-qualified (10 declarations, 12 `.sort()` call sites), so this class of failure is closed rather than merely worked around. A new comparison operator added without `const` reopens it, and no compiler warning catches that -- clang has no `-W` flag for it, and clang-tidy's `readability-make-member-function-const` is a separate tool the build does not run. The mechanical backstop is having a CI leg that builds with **libc++**, which today means the macOS leg. A `-stdlib=libc++` leg on Linux would catch it just as well.

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

GitHub Actions runs `./scripts/check.sh` on a **macOS + Linux matrix** (`macos-15`, `ubuntu-24.04`). Both jobs use `mise install` for cmake/ninja; Linux also installs `clang` from apt. Local Mac dev can use the same path via `.mise.toml`, or legacy `brew bundle`. Do **not** apt-install SDL2, OpenGL loaders, or OpenAL Soft on that job.

## check vs runtime

`check` is the gate. `runtime` is a local-only configure that **may** find window / GL / audio packages. Later GL link, SDL input, and OpenAL play slices use `runtime`; they must not add those packages to `check` or to CI apt.

| Preset | Who runs it | SDL2 / OpenGL / OpenAL |
|--------|-------------|------------------------|
| `check` (`./scripts/check.sh`, CI) | Everyone | **Not searched.** `RS2_RUNTIME=OFF`. Stubs only. |
| `runtime` | Local machine with optional packages | `find_package` **QUIET / not REQUIRED**. Missing package → feature off, configure still succeeds. |

This slice does **not** draw, poll SDL events, or play OpenAL. `cmake --preset runtime` only records `RS2_HAVE_SDL2` / `RS2_HAVE_OPENGL` / `RS2_HAVE_OPENAL` (`ON` or `OFF` in the configure log). Do not link `lib/graphic.cpp` / `lib/vertex.cpp` / `lib/sound.cpp` from this preset yet.

Optional local packages (Homebrew; not in `Brewfile`, not required for `check`):

```bash
brew install sdl2 openal-soft
# OpenAL Soft is keg-only on macOS; prefix it if FindOpenAL misses it:
cmake --preset runtime --fresh \
  -DCMAKE_PREFIX_PATH="$(brew --prefix openal-soft)"
```

Linux: install distro `libsdl2-dev`, OpenGL headers, and `libopenal-dev` the same way — locally, never as a `check` CI step. If nothing is installed, `cmake --preset runtime` must still succeed with all three features off.

See [adr-backend.md](adr-backend.md) for why the stack is SDL2 + GL 3.3 core / GLES3 + OpenAL Soft.

## Next milestones

- **#10** replaces the roundtrip passthrough with `CSaveFile` and drives byte-identity (`%p` / MD5 / float). The ctest harness itself is [#24](https://github.com/lollipop-onl/railsim2-portable/issues/24) ([rs2-roundtrip.md](rs2-roundtrip.md)).
- **#6** replaces `CXFile` / `D3DXLoadMeshFromX` with the closed parser in `port/xfile.cpp`. Loading Distribution `.x` into memory is [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) ([x-file-parser.md](x-file-parser.md)).
- Backend / window bring-up uses the `runtime` preset for optional SDL2 / GL / OpenAL (see `adr-backend.md`); keep `check` stub-only.
