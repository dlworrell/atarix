#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# ATARIX software test bench (offline). Runs, in order:
#   1. build: vector runner, CPU card simulator, test ROM, liar ROM
#   2. CPU card boot + mailbox test (CLK W65C816 + Northbridge model + libatarix)
#   3. CPU card negative controls (nine injected defects must each fail)
#   4. RTL co-simulation: CPU bus trace -> atx_northbridge_bus_shim.v (Icarus)
#   5. co-simulation negative control
#   6. co-simulation at datasheet bus timing (reported; gating once the shim is fixed)
# CPU instruction vectors need network access; run scripts/sim/run_cpu_vectors.sh.
# Artifacts: build/sim/ (report JSON, bus trace, co-sim report, summary.txt).

set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="${ATARIX_SIM_BUILD:-${root_dir}/build/sim}"
summary="${build_dir}/summary.txt"
mkdir -p "${build_dir}"
: > "${summary}"

step() {
    printf '\n== %s\n' "$1"
    printf '%s\n' "$1" >> "${summary}"
}

for tool in ca65 ld65 iverilog vvp python3; do
    command -v "${tool}" >/dev/null 2>&1 || { echo "missing tool: ${tool}" >&2; exit 2; }
done

step "build"
make -s -C "${root_dir}/sim" all BUILD="${build_dir}"

step "CPU card boot and mailbox test"
"${build_dir}/atarix-cpu-card" --rom "${build_dir}/cpu-card-test.rom" \
    --trace "${build_dir}/cpu-card-bus.trace" --report "${build_dir}/cpu-card-report.json" \
    | tee -a "${summary}"

step "CPU card negative controls"
"${root_dir}/sim/cpu-card/negative_control.sh" "${build_dir}/atarix-cpu-card" \
    "${build_dir}/cpu-card-test.rom" "${build_dir}/cpu-card-liar.rom" | tee -a "${summary}"

step "RTL co-simulation: bus shim"
python3 -I "${root_dir}/sim/rtl-cosim/run_bus_shim_cosim.py" \
    "${build_dir}/cpu-card-bus.trace" "${build_dir}/rtl-cosim" | tee -a "${summary}"

step "RTL co-simulation negative control"
python3 -I "${root_dir}/sim/rtl-cosim/run_bus_shim_cosim.py" \
    "${build_dir}/cpu-card-bus.trace" "${build_dir}/rtl-cosim-negative" --negative-control \
    | tee -a "${summary}"

step "RTL co-simulation at datasheet timing (reported finding, not yet gating)"
python3 -I "${root_dir}/sim/rtl-cosim/run_bus_shim_cosim.py" \
    "${build_dir}/cpu-card-bus.trace" "${build_dir}/rtl-cosim-timing" --datasheet-timing \
    | tee -a "${summary}"

step "software test bench: PASS"
