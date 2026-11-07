#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Copies the ATARIX machine driver into a MAME source tree and registers it
# in the driver list (ADR-0016). Usage: overlay.sh <mame-source-tree>
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
mame="${1:?usage: overlay.sh <mame-source-tree>}"
[[ -f "$mame/src/mame/mame.lst" ]] || { echo "error: $mame is not a MAME source tree" >&2; exit 1; }
mkdir -p "$mame/src/mame/atarix"
cp "$here/src/mame/atarix/atarix.cpp" "$mame/src/mame/atarix/atarix.cpp"
if ! grep -qx '@source:atarix/atarix.cpp' "$mame/src/mame/mame.lst"; then
    printf '\n@source:atarix/atarix.cpp\natarix\n' >> "$mame/src/mame/mame.lst"
fi
echo "ATARIX driver overlaid on $mame"
