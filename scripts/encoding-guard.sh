#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

python3 - <<'PY'
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path('.')
SOURCE_SUFFIXES = {'.cpp', '.h', '.H', '.c'}
errors: list[str] = []

# SJIS lead bytes (typical ranges)
def is_sjis_lead(b: int) -> bool:
    return (0x81 <= b <= 0x9F) or (0xE0 <= b <= 0xFC)

def is_sjis_trail(b: int) -> bool:
    return (0x40 <= b <= 0x7E) or (0x80 <= b <= 0xFC)

for path in sorted(p for p in ROOT.rglob('*') if p.suffix in SOURCE_SUFFIXES):
    rel = path.as_posix()
    if rel.startswith('.git/') or '/Distribution/' in rel or rel.startswith('port/stub/'):
        continue
    data = path.read_bytes()

    if data.startswith(b'\xef\xbb\xbf'):
        errors.append(f'{rel}: UTF-8 BOM detected (sources must stay CP932/ASCII)')

    # Heuristic UTF-8 text (excluding pure ASCII)
    if b'\x00' not in data:
        try:
            data.decode('utf-8')
            if any(b >= 0x80 for b in data):
                errors.append(f'{rel}: looks like UTF-8 text; expected CP932/ASCII source bytes')
        except UnicodeDecodeError:
            pass

    lines = data.splitlines()
    for i, line in enumerate(lines, 1):
        # SJIS second-byte 0x5C inside string literals
        in_string = False
        j = 0
        while j < len(line):
            c = line[j]
            if c == ord('"') and (j == 0 or line[j - 1] != ord('\\')):
                in_string = not in_string
                j += 1
                continue
            if in_string and is_sjis_lead(c) and j + 1 < len(line) and line[j + 1] == 0x5C:
                errors.append(
                    f'{rel}:{i}: SJIS trail 0x5C inside string literal (use wording change or split literals)'
                )
                break
            j += 1

        stripped = line.rstrip()
        if stripped.endswith(b'\\') and len(stripped) >= 2:
            prev = stripped[-2]
            if is_sjis_lead(prev):
                errors.append(f'{rel}:{i}: SJIS-induced line continuation (trail 0x5C at EOL)')

if errors:
    print('encoding-guard: FAILED', file=sys.stderr)
    for err in errors:
        print(err, file=sys.stderr)
    sys.exit(1)

print('encoding-guard: ok')
PY
