#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Fetch pinned vectors and run them against the vendored CLK cores.
#
# Usage: scripts/sim/run_cpu_vectors.sh [--full] [<set> ...]
#   default sets: 65816 6502 wdc65c02
#   default mode: the ATARIX sample (opcodes listed in sim/cpu/sample-opcodes.txt);
#   --full runs every opcode file in each set (several GB of downloads).
# Outputs: build/sim/vectors/<set>-report.json and a log per set.

set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="${ATARIX_SIM_BUILD:-${root_dir}/build/sim}"
vector_dir="${ATARIX_VECTOR_CACHE:-${root_dir}/build/vectors}"
out_dir="${build_dir}/vectors"
mode="sample"
sets=()

for arg in "$@"; do
    case "${arg}" in
        --full) mode="full" ;;
        -*) echo "unknown option ${arg}" >&2; exit 2 ;;
        *) sets+=("${arg}") ;;
    esac
done
[ "${#sets[@]}" -gt 0 ] || sets=(65816 6502 wdc65c02)

make -s -C "${root_dir}/sim" "${build_dir}/atarix-cpu-vectors" BUILD="${build_dir}"
mkdir -p "${out_dir}"

status=0
for set_name in "${sets[@]}"; do
    files=()
    if [ "${mode}" = "sample" ]; then
        while IFS= read -r line; do
            line="${line%%#*}"
            read -r key file <<< "${line}" || true
            [ -n "${key:-}" ] && [ "${key}" = "${set_name}" ] && files+=("${file}")
        done < "${root_dir}/sim/cpu/sample-opcodes.txt"
        if [ "${#files[@]}" -eq 0 ]; then
            echo "no sample opcodes listed for ${set_name}" >&2
            exit 2
        fi
    fi

    "${root_dir}/scripts/sim/fetch_cpu_vectors.sh" "${set_name}" "${vector_dir}" "${files[@]}"

    known="${root_dir}/sim/cpu/known-deviations/${set_name}.txt"
    known_args=()
    [ -f "${known}" ] && known_args=(--known "${known}")

    if "${build_dir}/atarix-cpu-vectors" --cpu "${set_name}" "${known_args[@]}" \
        --report "${out_dir}/${set_name}-${mode}-report.json" \
        "${vector_dir}/${set_name}"/*.json > "${out_dir}/${set_name}-${mode}.log" 2>&1; then
        tail -n 1 "${out_dir}/${set_name}-${mode}.log"
    else
        status=1
        cat "${out_dir}/${set_name}-${mode}.log"
    fi
done
exit "${status}"
