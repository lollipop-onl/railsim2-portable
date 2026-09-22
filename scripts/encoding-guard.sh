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

# Diffing each upstream file against 2324375 would be exact, but CI checks
# out at depth 1 and the Linux check runs on a `git archive` tree, so neither
# has that commit. These are the two shapes a cp1252 round-trip leaves
# (#50 / #54, fixed in #172), and both are absent from all of 2324375.
#
# cp1252 leaves 0x81 0x8D 0x8F 0x90 0x9D undefined and the round-trip folds
# all five into 0x9D. The result is still structurally valid SJIS, so only the
# lead byte gives it away: 2324375 has 52588 SJIS characters and no 0x9D lead.
CP1252_FOLDED_LEAD = 0x9D

def sjis_lead_bytes(line: bytes):
    j = 0
    while j < len(line):
        if is_sjis_lead(line[j]):
            yield line[j]
            j += 2
        else:
            j += 1

# A round-trip through an ASCII codec turns each CP932 character into '?'.
# Only comments are checked: Network.cpp keeps a literal "???" from upstream.
def comment_text(line: bytes) -> bytes:
    marker = line.find(b'//')
    if marker >= 0:
        return line[marker + 2:]
    stripped = line.lstrip()
    if stripped.startswith(b'/*') or stripped.startswith(b'*'):
        return stripped
    return b''

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
        if CP1252_FOLDED_LEAD in sjis_lead_bytes(line):
            errors.append(
                f'{rel}:{i}: SJIS lead byte 0x9D (cp1252 round-trip damage; restore the line from upstream 2324375)'
            )
        if b'??' in comment_text(line):
            errors.append(
                f'{rel}:{i}: "??" in a comment (CP932 text lost to "?"; restore the line from upstream 2324375)'
            )

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
