#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

ALLOWLIST="${ROOT}/port/native_sources.txt"

# The Linux leg of #124 measures a `git archive` extraction inside a container,
# where a bind mount would leak the macOS host's case-insensitivity. That tree
# has no `.git`, so the denominator cannot come from `git ls-files`.
total="$(find . lib -maxdepth 1 -type f -name '*.cpp' | wc -l | tr -d '[:space:]')"

enabled=0
while IFS= read -r line || [[ -n "${line}" ]]; do
  line="${line%%#*}"
  line="$(echo "${line}" | tr -d '[:space:]')"
  [[ -z "${line}" ]] && continue
  enabled=$((enabled + 1))
done < "${ALLOWLIST}"

python3 - <<PY
import json
print(json.dumps({
  "total": ${total},
  "enabled": ${enabled},
  "compiled": ${enabled},
  "ratio": "${enabled}/${total}",
  "message": "${enabled}/${total} game translation units compile natively"
}, ensure_ascii=False))
PY
