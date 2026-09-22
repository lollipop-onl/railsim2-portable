# railsim2-portable

Cross-platform port of [RailSim2](https://github.com/aizentranza/railsim2) (LGPL 2.1) -- a railway layout simulator originally for Windows / DirectX 8.

This repository tracks the portable build. Upstream is frozen, so what the port preserves is the game's observable behavior rather than upstream's text: platform code is replaced behind the UDX / DirectX-compatible layer in `lib/` and the compile stubs in `port/stub/`, and the on-disk formats are pinned by `check` tests.

## Credit

RailSim II was created by **Intaanetto Teiryuujo** (インターネット停留所).

- Copyright (C) 2003-2013 Intaanetto Teiryuujo
- Upstream: [aizentranza/railsim2](https://github.com/aizentranza/railsim2)
- License: GNU LGPL 2.1 (see [`LICENSE`](LICENSE) and [`NOTICE`](NOTICE))

This fork exists to run that work on additional platforms. It is not an official release from the original author.

## Platforms

| Platform | Status |
|----------|--------|
| Windows (original DirectX 8) | Upstream / reference |
| macOS | M1: compile allowlist + linked stub binary (`check` preset) |
| Linux | M1: same CI gate as macOS |
| Web (WebAssembly) | Later (optional milestone) |

Native linking, runtime bring-up, and a playable binary are **not** M0 goals. M1 links a stub `railsim2` executable; it does not start the game.

## Status

M1 (Surface): `./scripts/check.sh` still green. `cmake --build --preset check` links `railsim2` (POSIX `main` in `port/native_entry.cpp`). Allowlist progress is `port/native_sources.txt` (see `scripts/progress.sh`). Gameplay is later milestones.

## Quick start

```bash
mise install              # cmake, ninja, ccache from .mise.toml
./scripts/check.sh        # encoding -> cmake -> build -> ctest -> progress JSON
```

Homebrew fallback on macOS:

```bash
brew bundle --file Brewfile
./scripts/check.sh
```

Details: [docs/porting/dev-env.md](docs/porting/dev-env.md).

## Upstream remote

```bash
git remote add upstream https://github.com/aizentranza/railsim2.git
git fetch upstream
```

`upstream` is for attribution and historical comparison only. It receives no further commits, so there are no upstream changes to take. Lineage and the port strategies that stand in for a follow policy: [docs/porting/upstream.md](docs/porting/upstream.md).

## License and modification notice (LGPL 2.1 section 2)

This project is a modified LGPL 2.1 work based on RailSim2.

- Full license text: [`LICENSE`](LICENSE)
- Copyright and change notices with dates: [`NOTICE`](NOTICE)
- Git history remains the detailed record of each edit

Game sources are edited when the edit is mechanical, or when a `check` test covers the behavior it changes; everything else goes behind a `lib/` / `port/` seam (the rule and its reasons: Hard constraint 1 in [`AGENTS.md`](AGENTS.md)). Mechanical edits that have shipped include path separators, encoding-safe debug strings, a Clang-friendly declaration, a header renamed to the case its includers spell, `const` on an `operator<`, `(float)` casts on narrowing initializers, and an MSVC-only array-new respelled in standard form. Some larger edits shipped with a test in the same PR -- `lib/input.cpp` lost its DirectInput poll (`+42 / -372`) alongside `rs2_input_self_test`, and `lib/wave.cpp` lost `mmio*` alongside `rs2_wave_load_self_test` -- but not all of them: `lib/main.cpp`'s `timeGetTime` -> `std::chrono::steady_clock` swap, `lib/mutex.h`'s single-instance lock turned into a no-op, the capture no-op in `Capture.cpp` / `lib/movie.cpp`, and `SystemCover.cpp`'s dialog routing are examples that shipped without one. What is held fixed is behavior, not text: `rs2_roundtrip` keeps `Sample.rs2` byte-identical through Load/Save, and the float, MD5, `.x`, `.wav`, path and charset self-tests pin the rest. New port scaffolding lives under `port/`, `scripts/`, and CMake.

## Development

See [docs/porting/dev-env.md](docs/porting/dev-env.md) for presets, the compile firewall, and CI notes.

## Tracking

Work is organized with GitHub Milestones and Issues: https://github.com/lollipop-onl/railsim2-portable/milestones

Agents may start **only** issues labeled `agent-ready`, one PR each. Protocol: [`AGENTS.md`](AGENTS.md).
