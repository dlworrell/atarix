#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Negative control for the CPU vector runner: proves that a corrupted
# expectation is reported as a failure, so a passing run means something.
# Usage: sim/cpu/negative_control.sh <runner> <65816 vector file>

set -euo pipefail
runner="${1:?runner}"
source_file="${2:?65816 vector file}"
work="$(mktemp -d)"
trap 'rm -rf "${work}"' EXIT

# Clean baseline must pass.
"${runner}" --cpu 65816 --max-tests 50 "${source_file}" > "${work}/clean.log"

python3 -I - "${source_file}" "${work}" <<'EOF'
import json, sys
src, out = sys.argv[1], sys.argv[2]
tests = json.load(open(src))[:50]
for name, mutate in (
    ("register", lambda t: t["final"].__setitem__("a", (t["final"]["a"] + 1) & 0xFFFF)),
    ("ram", lambda t: t["final"]["ram"][0].__setitem__(1, (t["final"]["ram"][0][1] + 1) & 0xFF)),
    ("cycle", lambda t: t["cycles"][0].__setitem__(1, (t["cycles"][0][1] + 1) & 0xFF)),
    ("lines", lambda t: t["cycles"][0].__setitem__(2, "--------")),
):
    data = json.loads(json.dumps(tests))
    mutate(data[7])
    with open(f"{out}/{name}.json", "w") as handle:
        json.dump(data, handle)
EOF

for kind in register ram cycle lines; do
    if "${runner}" --cpu 65816 "${work}/${kind}.json" > "${work}/${kind}.log" 2>&1; then
        echo "negative control FAILED: corrupted ${kind} expectation was accepted" >&2
        exit 1
    fi
    if ! grep -q "failed=1 " "${work}/${kind}.log"; then
        echo "negative control FAILED: expected exactly one failure for ${kind}" >&2
        cat "${work}/${kind}.log" >&2
        exit 1
    fi
done
echo "negative control passed: register, ram, cycle and bus-line corruptions each detected"
