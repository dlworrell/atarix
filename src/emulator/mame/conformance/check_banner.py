#!/usr/bin/env python3
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
"""ATX-SPEC-022 conformance check 1 (boot banner), as far as milestone M0 reaches.

Reads a console transcript and requires the boot-sequence-v1 banner fields.
The reset reason is only required to be present: the supervisor mailbox that
reports it arrives in M2. Exits 0 on pass, 1 on failure.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

# EP-A RAM: $000000-$00BFFF (48 KiB) plus banks $01-$EF (239 x 64 KiB).
EP_A_RAM_KIB = 48 + 239 * 64

REQUIRED = [
    ("banner", r"^ATARIX ROM Monitor$"),
    ("ROM version", r"^ROM Version: \S+$"),
    ("reset reason", r"^Reset Reason: \S.*$"),
    ("CPU mode", r"^CPU Mode: native 65C816$"),
    ("board revision", r"^Board Revision: \$[0-9A-F]{2}$"),
    ("detected RAM", rf"^Detected RAM: {EP_A_RAM_KIB} KiB$"),
    ("ready", r"^Ready\.$"),
]


def check(text: str) -> list[str]:
    lines = text.replace("\r\n", "\n").split("\n")
    missing = []
    for name, pattern in REQUIRED:
        if not any(re.match(pattern, line) for line in lines):
            missing.append(f"{name} (/{pattern}/)")
    return missing


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("transcript", type=Path)
    args = parser.parse_args(argv)
    text = args.transcript.read_bytes().decode("ascii", "replace")
    print("--- console transcript ---")
    print(text)
    print("--------------------------")
    missing = check(text)
    if missing:
        for item in missing:
            print(f"FAIL: missing {item}")
        return 1
    print("PASS: ATX-SPEC-022 conformance check 1 (boot banner, M0 scope)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
