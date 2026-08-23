# railsim2-portable

Cross-platform port of [RailSim2](https://github.com/aizentranza/railsim2) (LGPL 2.1) ? a railway layout simulator originally for Windows / DirectX 8.

This repository tracks the portable build. Game logic stays as close to upstream as possible; platform code is replaced behind the UDX / DirectX-compatible layer in `lib/`.

## Status

M0 (Foundation): compile-firewall checks on macOS. Native linking and runtime are not goals yet.

## Quick start (macOS)

```bash
brew bundle --file Brewfile   # or: mise install
./scripts/check.sh
```

`scripts/check.sh` configures the `check` CMake preset, builds object files through `port/stub/`, runs encoding guards, and prints JSON progress from `scripts/progress.sh`.

## Upstream

- Original project: [aizentranza/railsim2](https://github.com/aizentranza/railsim2)
- License: GNU LGPL 2.1 (see `LICENSE`)

### Modification notice (LGPL 2.1 §2)

This fork modifies the original RailSim2 sources for cross-platform portability. Modified files include build scaffolding under `port/`, `scripts/`, `CMakeLists.txt`, path-normalization edits, and encoding-safe string literal adjustments. Each modified file should carry a brief change note in commit history.

## Development

See [docs/porting/dev-env.md](docs/porting/dev-env.md) for commands, presets, and fallback compiler notes.

## Tracking

Work is organized with GitHub Milestones and Issues only: https://github.com/lollipop-onl/railsim2-portable/milestones
