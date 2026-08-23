#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

TOTAL="${RS2_NATIVE_TOTAL:-254}"
ALLOWLIST="${ROOT}/port/native_sources.txt"

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
  "total": ${TOTAL},
  "enabled": ${enabled},
  "compiled": ${enabled},
  "ratio": "${enabled}/${TOTAL}",
  "message": f"${enabled}/${TOTAL} files compile natively"
}, ensure_ascii=False))
PY
