#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if [[ -f "${ROOT}/.mise.toml" ]] && command -v mise >/dev/null 2>&1; then
  for tool in cmake ninja; do
    if ! command -v "${tool}" >/dev/null 2>&1; then
      tool_dir="$(mise where "${tool}" 2>/dev/null || true)"
      if [[ -n "${tool_dir}" && -d "${tool_dir}/bin" ]]; then
        export PATH="${tool_dir}/bin:${PATH}"
      elif [[ -n "${tool_dir}" ]]; then
        export PATH="${tool_dir}:${PATH}"
      fi
    fi
  done
fi

failures=0

check() {
  if ! "$@"; then
    failures=$((failures + 1))
  fi
}

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "error: required command not found: $1" >&2
    exit 127
  fi
}

require_cmd cmake
require_cmd ninja
require_cmd python3

"${ROOT}/scripts/encoding-guard.sh"

cmake --preset check
cmake --build --preset check
ctest --preset check --output-on-failure

"${ROOT}/scripts/progress.sh"

if [[ "${failures}" -ne 0 ]]; then
  exit 1
fi

echo "check.sh: ok"
