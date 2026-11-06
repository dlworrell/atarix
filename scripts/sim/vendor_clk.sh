#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Re-vendor the CLK CPU-core subset into third_party/clk at a pinned commit.
# Usage: scripts/sim/vendor_clk.sh <path-to-CLK-checkout> <commit>
# Fails if the computed include closure differs from the committed file list,
# so a widened dependency set is reviewed instead of silently imported.

set -euo pipefail

clk_dir="${1:?usage: vendor_clk.sh <CLK checkout> <commit>}"
commit="${2:?usage: vendor_clk.sh <CLK checkout> <commit>}"
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
dest="${root_dir}/third_party/clk"

actual="$(git -C "${clk_dir}" rev-parse HEAD)"
if [ "${actual}" != "${commit}" ]; then
    echo "CLK checkout is at ${actual}, expected ${commit}" >&2
    exit 1
fi

closure="$(python3 -I - "${clk_dir}" <<'EOF'
import os, re, sys
root = sys.argv[1]
todo = ["Processors/6502Esque/6502Selector.hpp",
        "Processors/65816/Implementation/65816Base.cpp",
        "Processors/65816/Implementation/65816Storage.cpp",
        "Processors/6502/Implementation/6502Storage.cpp"]
seen = set()
while todo:
    f = todo.pop()
    if f in seen:
        continue
    seen.add(f)
    with open(os.path.join(root, f), encoding="utf-8", errors="replace") as handle:
        text = handle.read()
    for inc in re.findall(r'#include\s+"([^"]+)"', text):
        for cand in (os.path.normpath(os.path.join(os.path.dirname(f), inc)), os.path.normpath(inc)):
            if os.path.isfile(os.path.join(root, cand)):
                todo.append(cand)
                break
        else:
            sys.exit(f"unresolved include {inc} from {f}")
print("\n".join(sorted(seen)))
EOF
)"

expected="$(cd "${dest}" && find . -type f \( -name '*.hpp' -o -name '*.cpp' \) | sed 's#^\./##' | LC_ALL=C sort)"
if [ "${closure}" != "$(printf '%s\n' "${closure}" | LC_ALL=C sort)" ] || [ "${closure}" != "${expected}" ]; then
    echo "CLK include closure changed; review before vendoring:" >&2
    diff <(printf '%s\n' "${expected}") <(printf '%s\n' "${closure}") >&2 || true
    exit 1
fi

while IFS= read -r f; do
    mkdir -p "${dest}/$(dirname "${f}")"
    cp "${clk_dir}/${f}" "${dest}/${f}"
done <<< "${closure}"
cp "${clk_dir}/LICENCE" "${dest}/LICENCE"
echo "Vendored $(printf '%s\n' "${closure}" | wc -l) files from CLK ${commit}; update third_party/clk/README.md."
