#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Vendor the cglm headers used by the simulated GPU compute service into
# third_party/cglm at a pinned commit.
# Usage: scripts/sim/vendor_cglm.sh <path-to-cglm-checkout> <commit>
# The set is the textual #include closure of cglm/affine.h, mat4.h and vec3.h,
# following every include regardless of platform conditionals, so the vendored
# copy builds with whichever SIMD path (SSE, AVX, NEON, WASM or scalar) the
# host compiler selects.

set -euo pipefail

src="${1:?usage: vendor_cglm.sh <cglm checkout> <commit>}"
commit="${2:?usage: vendor_cglm.sh <cglm checkout> <commit>}"
root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
dest="${root_dir}/third_party/cglm"

actual="$(git -C "${src}" rev-parse HEAD)"
if [ "${actual}" != "${commit}" ]; then
    echo "cglm checkout is at ${actual}, expected ${commit}" >&2
    exit 1
fi

closure="$(python3 -I - "${src}/include" <<'EOF'
import os, re, sys
root = sys.argv[1]
todo = ["cglm/affine.h", "cglm/mat4.h", "cglm/vec3.h"]
seen = set()
while todo:
    f = todo.pop()
    if f in seen:
        continue
    seen.add(f)
    with open(os.path.join(root, f), encoding="utf-8") as handle:
        text = handle.read()
    for inc in re.findall(r'#\s*include\s+"([^"]+)"', text):
        cand = os.path.normpath(os.path.join(os.path.dirname(f), inc))
        if os.path.isfile(os.path.join(root, cand)):
            todo.append(cand)
        else:
            sys.exit(f"unresolved include {inc} from {f}")
print("\n".join(sorted(seen)))
EOF
)"

rm -rf "${dest}/cglm"
while IFS= read -r f; do
    mkdir -p "${dest}/$(dirname "${f}")"
    cp "${src}/include/${f}" "${dest}/${f}"
done <<< "${closure}"
cp "${src}/LICENSE" "${dest}/LICENSE"
echo "Vendored $(printf '%s\n' "${closure}" | wc -l) cglm headers from ${commit}; update third_party/cglm/README.md."
