#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Formal proof of rtl/atarix/atx_northbridge_bus_shim.v with the harness in
# sim/formal/bus_shim_formal.sv, using only Yosys (clk2fflogic for the two
# unrelated clocks, built-in SAT k-induction). No external solver needed.
#
# Runs (each must have the stated outcome, or the script fails):
#   1. PHI2 phases >= 4 steps (2 clk periods), latency <= 6 steps  PROVEN
#   2. PHI2 phases >= 3 steps: rate requirement violated            COUNTEREXAMPLE
#   3. latency bound 5 steps: shows 6 (3 clk periods) is tight      COUNTEREXAMPLE
#   4. mutant reading the live bus instead of the held capture       COUNTEREXAMPLE
# Artifacts: build/rtl/formal/<run>.log and summary.txt.

set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
out_dir="${root_dir}/build/rtl/formal"
mkdir -p "${out_dir}"
cd "${root_dir}"
command -v yosys >/dev/null 2>&1 || { echo "missing tool: yosys" >&2; exit 127; }

summary="${out_dir}/summary.txt"
: > "${summary}"
status=0

run() {
    local name="$1" expect="$2" defines="$3"
    local log="${out_dir}/${name}.log"
    local result
    timeout 900 yosys -q -l "${log}" -p "read_verilog -formal -DFORMAL ${defines} \
        rtl/atarix/atx_northbridge_bus_shim.v sim/formal/bus_shim_formal.sv; \
        prep -top bus_shim_formal; flatten; clk2fflogic; opt_clean; \
        sat -tempinduct -prove-asserts -set-assumes -set-init-zero -seq 1 -maxsteps 30 -timeout 600 bus_shim_formal" \
        > /dev/null 2>&1 || true
    if grep -q "Induction step proven: SUCCESS" "${log}"; then
        result="PROVEN"
    elif grep -q "model found for base case: FAIL" "${log}"; then
        result="COUNTEREXAMPLE"
    else
        result="UNDECIDED"
    fi
    if [ "${result}" = "${expect}" ]; then
        echo "ok   ${name}: ${result}" | tee -a "${summary}"
    else
        echo "FAIL ${name}: ${result}, expected ${expect}" | tee -a "${summary}"
        status=1
    fi
}

run proof           PROVEN         "-DPHI2_MIN_HALF=4 -DMAX_LATENCY=6"
run rate-violation  COUNTEREXAMPLE "-DPHI2_MIN_HALF=3 -DMAX_LATENCY=6"
run latency-tight   COUNTEREXAMPLE "-DPHI2_MIN_HALF=4 -DMAX_LATENCY=5"
run mutant-livedata COUNTEREXAMPLE "-DPHI2_MIN_HALF=4 -DMAX_LATENCY=6 -DSHIM_MUTANT_LIVE_DATA"

exit "${status}"
