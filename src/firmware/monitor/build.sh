#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Builds the 64 KiB ATARIX boot ROM with cc65's ca65 and ld65.
# Usage: build.sh [output-directory]   (default: build/monitor)
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
out="${1:-build/monitor}"
mkdir -p "$out"
ca65 --cpu 65816 -l "$out/monitor.lst" -o "$out/monitor.o" "$here/monitor.s"
ld65 -C "$here/monitor.cfg" -m "$out/monitor.map" -o "$out/atarix-monitor.bin" "$out/monitor.o"
size="$(stat -c %s "$out/atarix-monitor.bin")"
if [[ "$size" != 65536 ]]; then
    echo "error: ROM image is $size bytes, expected 65536" >&2
    exit 1
fi
echo "$out/atarix-monitor.bin (65536 bytes)"
