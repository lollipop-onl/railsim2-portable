# Upstream remote and follow policy

## Remotes

| Remote   | URL                                              | Role                                      |
|----------|--------------------------------------------------|-------------------------------------------|
| `origin` | `git@github.com:lollipop-onl/railsim2-portable.git` | This portable fork (default push/pull)  |
| `upstream` | `https://github.com/aizentranza/railsim2.git`  | Original RailSim2 sources (lineage only)  |

Add `upstream` once per clone (remotes are not committed):

```bash
git remote add upstream https://github.com/aizentranza/railsim2.git
git fetch upstream
```

`upstream` is for attribution and historical comparison. The upstream project is not expected to receive further commits or tags, so there is no routine fetch / merge / tag-watch workflow.

## Lineage

Git history on this fork still contains the upstream drops:

- `9c90bc9` Initial commit
- `2662b8a` Version 2.12 source code
- `2324375` Version 2.15 source code

Port work starts after `2324375` (see `NOTICE` for modification dates).

## Follow policy

We do not track upstream for ongoing sync. The living rules are the **port strategies already decided for this fork**. When any of these change, update this section in the same change (do not leave stale strategy here).

Current strategies:

1. **Game logic stays close to upstream** -- prefer stubs / `lib/` backends over rewriting gameplay.
2. **No MinGW and no vendored DirectX SDK** -- compile firewall via `port/stub/` instead.
3. **Sources stay CP932 / ASCII** -- encoding-guard enforces; no mass UTF-8 conversion of game sources.
4. **Progress is monotonic** via `port/native_sources.txt` (denominator 254).
5. **Single gate** -- `./scripts/check.sh` is the local and CI truth for M0+.

There is no separate denylist of "do not touch" paths. Scope is defined by the strategies above and by the active Milestone / Issue, not by a frozen exclusion list.

## Related

- License text: [`LICENSE`](../../LICENSE)
- Change log for LGPL notices: [`NOTICE`](../../NOTICE)
- Credit and overview: [`README.md`](../../README.md)
- Dev environment: [`dev-env.md`](dev-env.md)
- Frozen udx / D3D8 game ABI: [`api-surface.md`](api-surface.md)
- Backend ADR: [`adr-backend.md`](adr-backend.md)
- X-File templates in `Distribution`: [`x-file-templates.md`](x-file-templates.md)
- Closed text `.x` parser: [`x-file-parser.md`](x-file-parser.md)
- `.rs2` roundtrip ctest: [`rs2-roundtrip.md`](rs2-roundtrip.md)
