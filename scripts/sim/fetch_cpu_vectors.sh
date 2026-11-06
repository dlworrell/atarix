#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Fetch pinned CPU test vectors without vendoring them (the sets are GB-sized).
#
# Usage: scripts/sim/fetch_cpu_vectors.sh <set> <dest-dir> [opcode-file ...]
#   <set>: 65816 | wdc65c02 | rockwell65c02 | synertek65c02 | 6502 | nes6502
#   opcode files are names inside the set's v1/ directory (e.g. fb.n.json, a9.json);
#   omit them to fetch the whole set.
#
# Integrity: each set is fetched at a pinned commit with a blob-less partial
# clone and sparse checkout. Git verifies every blob against the object IDs in
# that commit's tree, so the bytes are exactly those of the pinned revision.
# The script fails if the fetched HEAD is not the pinned commit.

set -euo pipefail

set_name="${1:?usage: fetch_cpu_vectors.sh <set> <dest-dir> [opcode-file ...]}"
dest="${2:?usage: fetch_cpu_vectors.sh <set> <dest-dir> [opcode-file ...]}"
shift 2

case "${set_name}" in
    65816)
        url="https://github.com/SingleStepTests/65816"
        commit="307c63f5673c8c6b861c52f71ddb89212a227480"
        subdir="v1"
        ;;
    wdc65c02|rockwell65c02|synertek65c02|6502|nes6502)
        url="https://github.com/dlworrell/65x02"
        commit="66ea3ef522d587c13ecf74b5349bcc8eba6bb57e"
        subdir="${set_name}/v1"
        ;;
    *)
        echo "unknown vector set: ${set_name}" >&2
        exit 2
        ;;
esac

repo="${dest}/.${set_name}-repo"
mkdir -p "${dest}"
if [ ! -d "${repo}/.git" ]; then
    git init -q "${repo}"
    git -C "${repo}" remote add origin "${url}"
fi
git -C "${repo}" config core.sparseCheckout true
git -C "${repo}" sparse-checkout init --no-cone >/dev/null

if [ "$#" -eq 0 ]; then
    printf '/%s/\n/LICENCE\n/LICENSE\n/README.md\n' "${subdir}" > "${repo}/.git/info/sparse-checkout"
else
    {
        printf '/LICENCE\n/LICENSE\n/README.md\n'
        for f in "$@"; do
            case "${f}" in
                */*|*..*) echo "opcode file must be a bare name: ${f}" >&2; exit 2 ;;
            esac
            printf '/%s/%s\n' "${subdir}" "${f}"
        done
    } > "${repo}/.git/info/sparse-checkout"
fi

git -C "${repo}" fetch -q --depth 1 --filter=blob:none origin "${commit}"
git -C "${repo}" -c advice.detachedHead=false checkout -q --force FETCH_HEAD
git -C "${repo}" sparse-checkout reapply

actual="$(git -C "${repo}" rev-parse HEAD)"
if [ "${actual}" != "${commit}" ]; then
    echo "fetched ${actual}, expected pinned ${commit}" >&2
    exit 1
fi

count="$(find "${repo}/${subdir}" -name '*.json' | wc -l)"
if [ "$#" -ne 0 ] && [ "${count}" -ne "$#" ]; then
    echo "requested $# vector files but found ${count}" >&2
    exit 1
fi

ln -sfn "${repo}/${subdir}" "${dest}/${set_name}"
echo "${set_name}: ${count} vector files at ${commit} -> ${dest}/${set_name}"
