#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Fit and timing check of ATARIX RTL on the Fabric Northbridge FPGA chosen in
# ADR-0001: Lattice ECP5 LFE5U-85F-6BG381C as fitted to the ULX3S
# (dlworrell/ulx3s 14cac31, README "Features").
#
# Each top is synthesized with yosys (synth_ecp5) and placed and routed with
# nextpnr-ecp5 in out-of-context mode: the logic is placed on the real device
# without I/O pins, because no board pinout for these modules is decided yet
# (the ATX-SPEC-020 wrapper alone has more ports than the ULX3S has GPIO).
# Timing target: the ULX3S 25 MHz oscillator (FREQUENCY PORT "clk_25mhz" 25 MHZ
# in doc/constraints/ulx3s_v20.lpf at that commit).
#
# Fails if any top does not route or misses 25 MHz.
# Artifacts: build/rtl/ecp5/<top>-report.json, logs, summary.txt.

set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
out_dir="${root_dir}/build/rtl/ecp5"
target_mhz="${ATARIX_ECP5_TARGET_MHZ:-25}"  # override only for the negative control
mkdir -p "${out_dir}"
cd "${root_dir}"

for tool in yosys nextpnr-ecp5 python3; do
    command -v "${tool}" >/dev/null 2>&1 || { echo "missing tool: ${tool}" >&2; exit 127; }
done

mapfile -t sources < <(find rtl/atarix -name '*.v' ! -name '*_tb.v' | LC_ALL=C sort)
tops=(atx_northbridge_bus_shim atx_spec_020_system_wrapper)
summary="${out_dir}/summary.txt"
: > "${summary}"
status=0

for top in "${tops[@]}"; do
    echo "== ${top}: LFE5U-85F CABGA381 speed 6, target ${target_mhz} MHz"
    if ! yosys -q -l "${out_dir}/${top}-yosys.log" \
        -p "read_verilog ${sources[*]}; synth_ecp5 -top ${top} -json ${out_dir}/${top}.json"; then
        echo "FAIL ${top}: synthesis" | tee -a "${summary}"
        status=1
        continue
    fi
    if ! nextpnr-ecp5 --85k --package CABGA381 --speed 6 --out-of-context \
        --json "${out_dir}/${top}.json" --freq "${target_mhz}" \
        --report "${out_dir}/${top}-report.json" -l "${out_dir}/${top}-pnr.log" > /dev/null 2>&1; then
        echo "FAIL ${top}: place and route or timing (see ${out_dir}/${top}-pnr.log)" | tee -a "${summary}"
        status=1
        continue
    fi
    python3 -I - "${top}" "${out_dir}/${top}-report.json" "${target_mhz}" <<'EOF' | tee -a "${summary}"
import json, sys
top, path, target = sys.argv[1], sys.argv[2], float(sys.argv[3])
report = json.load(open(path))
fmax = min(v["achieved"] for v in report["fmax"].values())
used = {k: v for k, v in report["utilization"].items() if v["used"]}
parts = ", ".join(f"{k} {v['used']}/{v['available']}" for k, v in sorted(used.items()))
verdict = "PASS" if fmax >= target else "FAIL"
print(f"{verdict} {top}: fmax {fmax:.1f} MHz (target {target:.0f}); {parts}")
sys.exit(0 if fmax >= target else 1)
EOF
    [ "${PIPESTATUS[0]}" -eq 0 ] || status=1
done

exit "${status}"
