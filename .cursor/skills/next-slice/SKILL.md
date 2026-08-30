---
name: next-slice
description: >-
  Picks exactly one GitHub issue labeled agent-ready, implements that slice,
  opens a PR, and stops. Use when the user says Ÿ, Ÿ‚â‚Á‚Ä, Ÿ‚Ìì‹Æ, pick next,
  continue the port, or asks to start work without naming an issue.
---

# Next slice

Follow `AGENTS.md` section "Agent queue". This skill is the entry point when the user does not name an issue.

## Steps

1. Run `gh issue list --label agent-ready --state open --json number,title,labels`.
2. If the list is empty: stop. Tell the user the queue is empty. Optionally `gh issue list --search "Parent: in:body state:open"` and show up to three slice candidates **without starting them**.
3. If several are ready: take the **lowest number**, unless its files clearly collide with an open PR (`gh pr list`). Skip colliding issues rather than starting them.
4. `gh issue comment <n> --body "Starting this slice. Removing \`agent-ready\` so another agent does not take it."`
5. `gh issue edit <n> --remove-label agent-ready`
6. Implement only that issue's ‚â‚é‚±‚Æ / Š®—¹ğŒ.
7. Run `./scripts/check.sh`.
8. Open one PR that `Closes #<slice>` (not the parent epic).
9. Stop. Do not look for the next issue.

## Do not

- Do not keep going after the PR exists.
- Do not start `#1`?`#17` as if they were one PR.
- Do not create a markdown TODO list as a second queue.
