# Agent notes

Cross-platform port of RailSim2. Game logic stays close to upstream; platform code is replaced behind `lib/` and `port/stub/`.

Human-facing overview: [`README.md`](README.md). Strategies: [`docs/porting/upstream.md`](docs/porting/upstream.md). Build: [`docs/porting/dev-env.md`](docs/porting/dev-env.md).

## Hard constraints

1. Prefer stubs and `lib/` backends over rewriting gameplay.
2. No MinGW and no vendored DirectX SDK. Compile firewall is `port/stub/` via `-isystem`.
3. Sources stay CP932 / ASCII. No mass UTF-8 conversion of game sources. `scripts/encoding-guard.sh` is law.
4. Progress is monotonic: only add lines to `port/native_sources.txt` (denominator 254).
5. `./scripts/check.sh` is the local and CI gate.

Scope is the active Milestone / Issue, not a frozen exclusion list.

## Commands

```bash
mise install            # cmake, ninja, ccache (see .mise.toml)
./scripts/check.sh      # encoding-guard -> cmake --preset check -> ctest -> progress JSON
```

Linux CI also installs `clang` from apt. Do not add SDL2 or other runtime deps to the `check` preset.

## Agent queue

Tracking remains GitHub Issues and Milestones. The queue is the `agent-ready` label.

Parent issues `#1`?`#17` are epics. They are **not** a session of work. Only a slice issue (body starts with `Parent: #N`) may be labeled `agent-ready`.

### When the user does not name an issue

("éü" / "éüÇ‚Ç¡Çƒ" / "pick next" / "continue")

1. `gh issue list --label agent-ready --state open`
2. **Zero issues:** stop. Say the queue is empty. List at most three unlabeled slice issues that look next, and wait. Do **not** start an unlabeled issue, a parent epic, or an open PR's remaining work.
3. **One or more:** take **one** issue (lowest number). Comment that you started it, then **remove** `agent-ready` so a second agent cannot take it.
4. Implement that slice only. Open one PR that `Closes #N` the slice (not the parent, unless the slice *is* the last remaining work and the parent checklist is done).
5. **Stop.** Do not pick a second issue in the same session. Do not "keep going."

### When the user names an issue

Follow that issue. If it is a parent epic, implement **one** unchecked slice-sized checklist item, then stop and say what remains. Do not finish an epic in one PR.

### Parallelism

Multiple `agent-ready` issues means they do not share files. If two ready issues would touch the same path, do the lower number and leave the other labeled.

Open PRs are in-flight work. Do not start a second implementation of the same parent.

## Slice size

One PR is one of:

- A docs/ADR/inventory that another issue can depend on
- One backend or OS seam (for example `timeGetTime` Å® chrono), with tests if they exist
- An allowlist expansion: add the smallest set of `port/native_sources.txt` entries that share one missing type or header, grow stubs as needed, keep `./scripts/check.sh` green

A slice is too big if it spans two Milestones, rewrites game logic, or cannot be verified with `./scripts/check.sh` plus the issue's own äÆóπèåè.

## Pull requests

- Keep game-code diffs mechanical (path separators, encoding-safe strings). Put behavior in `lib/` or `port/`.
- `./scripts/check.sh` must be green.
- Do not chain a second issue after CI goes green.

## Cursor Cloud specific instructions

Cloud VMs are Linux. After install, `clang`, `cmake`, `ninja`, and `python3` must exist, then `./scripts/check.sh` must pass on the branch you started from.

```bash
sudo apt-get update && sudo apt-get install -y clang cmake ninja-build python3
./scripts/check.sh
```

`gh` is the GitHub client. Do not invent a local markdown queue.
