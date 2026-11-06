#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# ATARIX software test bench (offline). Runs, in order:
#   1. build: vector runner, CPU card simulator, test ROM, liar ROM
#   2. CPU card boot + mailbox test (CLK W65C816 + Northbridge model + libatarix)
#   3. CPU card negative controls (seventeen injected defects must each fail)
#   4. RTL co-simulation: CPU bus trace -> atx_northbridge_bus_shim.v ->
#      atx_mailbox_tx_assembler.v (Icarus); every mailbox write, every frame
#      and the C Northbridge audit (type, sequence per SUBMIT) must match
#   5. co-simulation negative controls (hidden data write, hidden SUBMIT)
#   6. co-simulation at W65C816 datasheet timing, gating: 4 MHz PHI2 on the
#      25 MHz ULX3S clock and 14 MHz PHI2 on a 62.5 MHz PLL clock must capture
#      every byte; 14 MHz on 25 MHz (below the proven 4:1 clock ratio) must fail
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

step "RTL co-simulation: bus shim + TX frame assembler (functional timing)"
python3 -I "${root_dir}/sim/rtl-cosim/run_bus_shim_cosim.py" "${build_dir}/cpu-card-bus.trace" \
    "${build_dir}/rtl-cosim" --audit-report="${build_dir}/cpu-card-report.json" | tee -a "${summary}"

step "RTL co-simulation negative controls: hidden data write, hidden SUBMIT"
python3 -I "${root_dir}/sim/rtl-cosim/run_bus_shim_cosim.py" "${build_dir}/cpu-card-bus.trace" \
    "${build_dir}/rtl-cosim-negative" --audit-report="${build_dir}/cpu-card-report.json" --negative-control | tee -a "${summary}"
python3 -I "${root_dir}/sim/rtl-cosim/run_bus_shim_cosim.py" "${build_dir}/cpu-card-bus.trace" \
    "${build_dir}/rtl-cosim-negative-submit" --audit-report="${build_dir}/cpu-card-report.json" --negative-control-submit | tee -a "${summary}"

step "RTL co-simulation at datasheet timing: 4 MHz PHI2, 25 MHz clock"
python3 -I "${root_dir}/sim/rtl-cosim/run_bus_shim_cosim.py" "${build_dir}/cpu-card-bus.trace" \
    "${build_dir}/rtl-cosim-timing-4mhz" --audit-report="${build_dir}/cpu-card-report.json" --datasheet-timing --phi2-mhz=4 --clk-mhz=25 \
    | tee -a "${summary}"

step "RTL co-simulation at datasheet timing: 14 MHz PHI2, 62.5 MHz PLL clock"
python3 -I "${root_dir}/sim/rtl-cosim/run_bus_shim_cosim.py" "${build_dir}/cpu-card-bus.trace" \
    "${build_dir}/rtl-cosim-timing-14mhz" --audit-report="${build_dir}/cpu-card-report.json" --datasheet-timing --phi2-mhz=14 --clk-mhz=62.5 \
    | tee -a "${summary}"

step "RTL co-simulation timing negative control: 14 MHz PHI2 on the 25 MHz clock"
python3 -I "${root_dir}/sim/rtl-cosim/run_bus_shim_cosim.py" "${build_dir}/cpu-card-bus.trace" \
    "${build_dir}/rtl-cosim-timing-negative" --audit-report="${build_dir}/cpu-card-report.json" --datasheet-timing --phi2-mhz=14 --clk-mhz=25 \
    --expect-timing-failure | tee -a "${summary}"

step "software test bench: PASS"
