# railsim2-portable

Cross-platform port of [RailSim2](https://github.com/aizentranza/railsim2) (LGPL 2.1) -- a railway layout simulator originally for Windows / DirectX 8.

This repository tracks the portable build. Game logic stays as close to upstream as possible; platform code is replaced behind the UDX / DirectX-compatible layer in `lib/`.

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

Follow policy (when and how to take upstream changes): [docs/porting/upstream.md](docs/porting/upstream.md).

## License and modification notice (LGPL 2.1 section 2)

This project is a modified LGPL 2.1 work based on RailSim2.

- Full license text: [`LICENSE`](LICENSE)
- Copyright and change notices with dates: [`NOTICE`](NOTICE)
- Git history remains the detailed record of each edit

Modified upstream files so far are limited to path separators, encoding-safe debug strings, and a Clang-friendly declaration -- plus new port scaffolding under `port/`, `scripts/`, and CMake. Game logic is intentionally left alone so diffs against upstream stay small and reviewable.

## Development

See [docs/porting/dev-env.md](docs/porting/dev-env.md) for presets, the compile firewall, and CI notes.

## Tracking

Work is organized with GitHub Milestones and Issues only: https://github.com/lollipop-onl/railsim2-portable/milestones
