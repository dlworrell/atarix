#!/usr/bin/env python3
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
"""Co-simulate the Northbridge bus shim and TX frame assembler RTL against the
C CPU-card model.

Reads the CPU card's bus trace (sim/cpu-card), drives every CPU bus cycle into
rtl/atarix/atx_northbridge_bus_shim.v -> rtl/atarix/atx_mailbox_tx_assembler.v
under Icarus Verilog, and requires:

  writes  every write to the $00D2xx mailbox block, address and data, in order;
  frames  every frame the assembler emits equals the TX window at that SUBMIT
          (16-byte header plus the claimed payload, capped at 48 bytes);
  audit   with --audit-report, the frames' message types and sequence numbers
          equal the decisions in the C Northbridge's audit log, one per SUBMIT;
  overrun the assembler never sees a SUBMIT while still streaming.

Usage: run_bus_shim_cosim.py <trace> <work-dir> [--audit-report=FILE]
       [--negative-control | --negative-control-submit]
       [--datasheet-timing [--phi2-mhz=F] [--clk-mhz=F] [--expect-timing-failure]]

  --negative-control         hide one TX-window data write from the RTL;
  --negative-control-submit  hide one SUBMIT from the RTL;
  --expect-timing-failure    the run is outside the proven clock ratio;
each of these requires the comparison to FAIL (exit 0 when it does).
Exit: 0 pass (or expected failure seen), 1 mismatch, 2 setup error.
"""

import json
import pathlib
import shutil
import subprocess
import sys

MAILBOX_BLOCK = range(0x00D200, 0x00D300)
TX_BASE = 0x00D280
WINDOW = 64
HEADER = 16
MB_CONTROL = 0x00D201
CONTROL_SUBMIT = 0x01


def expected_from_trace(trace_path, hide_data, hide_submit):
    stimulus, writes, frames = [], [], []
    window = [0] * WINDOW
    hidden = False
    with trace_path.open() as handle:
        for line in handle:
            parts = line.split()
            if len(parts) != 4:
                continue
            _cycle, kind, address_text, data_text = parts
            address = int(address_text, 16)
            data = int(data_text, 16)
            is_write = kind == "W"
            selected = address in MAILBOX_BLOCK
            cs_n = 0 if selected else 1
            in_window = TX_BASE <= address < TX_BASE + WINDOW
            is_submit = is_write and address == MB_CONTROL and data == CONTROL_SUBMIT
            # Negative controls: the RTL never sees one write the C model saw.
            if not hidden and is_write and selected:
                if (hide_data and in_window and len(writes) >= 40) or (hide_submit and is_submit and frames):
                    cs_n = 1
                    hidden = True
            stimulus.append(((address & 0xFF) << 10) | (cs_n << 9) | ((0 if is_write else 1) << 8) | data)
            if is_write and selected:
                writes.append((address & 0xFF, data))
                if in_window:
                    window[address - TX_BASE] = data
                if is_submit:
                    claimed = window[6] | (window[7] << 8)
                    frames.append(window[:HEADER + min(claimed, WINDOW - HEADER)])
    return stimulus, writes, frames


def main(argv):
    if len(argv) < 3:
        print(__doc__, file=sys.stderr)
        return 2
    trace_path = pathlib.Path(argv[1])
    work = pathlib.Path(argv[2])
    flags = argv[3:]
    hide_data = "--negative-control" in flags
    hide_submit = "--negative-control-submit" in flags
    timing = "--datasheet-timing" in flags
    expect_timing_fail = "--expect-timing-failure" in flags
    phi2_mhz, clk_mhz, audit_path = 4.0, 25.0, None
    for arg in flags:
        if arg.startswith("--phi2-mhz="):
            phi2_mhz = float(arg.split("=", 1)[1])
        elif arg.startswith("--clk-mhz="):
            clk_mhz = float(arg.split("=", 1)[1])
        elif arg.startswith("--audit-report="):
            audit_path = pathlib.Path(arg.split("=", 1)[1])
    root = pathlib.Path(__file__).resolve().parents[2]
    work.mkdir(parents=True, exist_ok=True)

    for tool in ("iverilog", "vvp"):
        if shutil.which(tool) is None:
            print(f"{tool} not found on PATH", file=sys.stderr)
            return 2

    stimulus, expected_writes, expected_frames = expected_from_trace(trace_path, hide_data, hide_submit)
    if not stimulus:
        print("empty trace", file=sys.stderr)
        return 2

    stim_file = work / "bus_shim_stimulus.hex"
    stim_file.write_text("".join(f"{word:08x}\n" for word in stimulus))
    capture_file = work / "bus_shim_capture.txt"
    binary = work / "bus_shim_cosim_tb.vvp"

    build = subprocess.run(
        ["iverilog", "-g2012", "-Wall", "-o", str(binary),
         str(root / "sim/rtl-cosim/bus_shim_cosim_tb.v"),
         str(root / "rtl/atarix/atx_northbridge_bus_shim.v"),
         str(root / "rtl/atarix/atx_mailbox_tx_assembler.v")],
        capture_output=True, text=True)
    if build.returncode != 0:
        print(build.stdout + build.stderr, file=sys.stderr)
        return 2

    run = subprocess.run(
        ["vvp", "-n", str(binary), f"+stimulus={stim_file}", f"+output={capture_file}",
         f"+cycles={len(stimulus)}"]
        + ([f"+timing=1", f"+clk_half_ps={round(500000 / clk_mhz)}",
            f"+phi2_half_ps={round(500000 / phi2_mhz)}"] if timing else []),
        capture_output=True, text=True, timeout=900)
    print(run.stdout.strip().splitlines()[0] if run.stdout.strip() else "")
    # vvp reports file errors as "ERROR:" lines yet still exits 0; treat them,
    # a testbench FAIL, or a missing capture file as a setup failure.
    if run.returncode != 0 or "FAIL" in run.stdout or "ERROR" in run.stdout or not capture_file.exists():
        print(run.stdout + run.stderr, file=sys.stderr)
        print("co-simulation did not run to completion", file=sys.stderr)
        return 2
    overrun = "overrun=1" in run.stdout

    rtl_writes, rtl_frames = [], []
    for line in capture_file.read_text().splitlines():
        parts = line.split()
        if parts and parts[0] == "W":
            rtl_writes.append((int(parts[1], 16), int(parts[2], 16)))
        elif parts and parts[0] == "F":
            rtl_frames.append([int(x, 16) for x in parts[1:]])

    writes_match = rtl_writes == expected_writes
    frames_match = rtl_frames == expected_frames
    good_writes = sum(1 for got, want in zip(rtl_writes, expected_writes) if got == want)

    audit_match = None
    if audit_path is not None:
        audit = json.loads(audit_path.read_text())["audit"]
        decoded = [(f[4] | (f[5] << 8), f[8] | (f[9] << 8) | (f[10] << 16) | (f[11] << 24)) for f in rtl_frames]
        audit_match = decoded == [(a["type"], a["sequence"]) for a in audit]

    ok = writes_match and frames_match and not overrun and audit_match is not False
    report = {
        "cpu_cycles": len(stimulus),
        "datasheet_timing": timing,
        "phi2_mhz": phi2_mhz if timing else None,
        "clk_mhz": clk_mhz if timing else None,
        "c_model_writes": len(expected_writes),
        "rtl_writes": len(rtl_writes),
        "rtl_writes_correct": good_writes,
        "writes_match": writes_match,
        "c_model_frames": len(expected_frames),
        "rtl_frames": len(rtl_frames),
        "frames_match": frames_match,
        "audit_match": audit_match,
        "overrun": overrun,
        "result": "pass" if ok else "mismatch",
    }
    (work / "bus_shim_cosim_report.json").write_text(json.dumps(report, indent=2) + "\n")

    label = f"{clk_mhz:g} MHz clock, {phi2_mhz:g} MHz PHI2, 10 ns hold" if timing else "functional timing"
    print(f"[{label}] writes: {good_writes}/{len(expected_writes)} correct "
          f"({len(rtl_writes)} captured) -> {'MATCH' if writes_match else 'MISMATCH'}")
    print(f"[{label}] frames: RTL {len(rtl_frames)} vs C model {len(expected_frames)} SUBMITs -> "
          f"{'MATCH' if frames_match else 'MISMATCH'}"
          + ("" if audit_match is None else f"; audit {'MATCH' if audit_match else 'MISMATCH'}")
          + ("; OVERRUN" if overrun else ""))

    if hide_data or hide_submit or expect_timing_fail:
        what = ("hidden data write" if hide_data else "hidden SUBMIT" if hide_submit
                else "out-of-spec clock ratio")
        if ok:
            print(f"negative control FAILED: {what} was not detected", file=sys.stderr)
            return 1
        print(f"negative control passed: {what} detected")
        return 0
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
