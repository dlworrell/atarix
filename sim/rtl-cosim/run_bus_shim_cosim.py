#!/usr/bin/env python3
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
"""Co-simulate the Northbridge bus shim RTL against the C CPU-card model.

Reads the CPU card's bus trace (sim/cpu-card), drives every CPU bus cycle into
rtl/atarix/atx_northbridge_bus_shim.v under Icarus Verilog, and compares:

  bytes   The RTL must capture exactly the bytes the C model received in its
          mailbox TX window, in order (pass/fail).
  frames  Frame boundaries: the C model frames on each SUBMIT; the shim marks a
          start-of-frame (SOF) after chip select deasserts. Both counts are
          reported. A mismatch is recorded as a known finding, not a pass,
          until the shim's framing rule is specified (see the test-bench doc).

Usage: run_bus_shim_cosim.py <trace> <work-dir> [--expect-sof-per-frame] [--negative-control]
       --negative-control hides one TX-window write from the RTL (deasserts its
       chip select) and requires the byte comparison to report a mismatch.
Exit:  0 bytes match (and frames, if --expect-sof-per-frame), 1 mismatch, 2 setup error.
"""

import json
import pathlib
import shutil
import subprocess
import sys

TX_WINDOW = range(0x00D280, 0x00D2C0)
MB_CONTROL = 0x00D201
CONTROL_SUBMIT = 0x01


def main(argv):
    if len(argv) < 3:
        print(__doc__, file=sys.stderr)
        return 2
    trace_path = pathlib.Path(argv[1])
    work = pathlib.Path(argv[2])
    strict_frames = "--expect-sof-per-frame" in argv[3:]
    negative = "--negative-control" in argv[3:]
    hidden = False
    root = pathlib.Path(__file__).resolve().parents[2]
    work.mkdir(parents=True, exist_ok=True)

    for tool in ("iverilog", "vvp"):
        if shutil.which(tool) is None:
            print(f"{tool} not found on PATH", file=sys.stderr)
            return 2

    stimulus = []
    expected_bytes = []
    frames = []
    current = []
    with trace_path.open() as handle:
        for line in handle:
            parts = line.split()
            if len(parts) != 4:
                continue
            _cycle, kind, address_text, data_text = parts
            address = int(address_text, 16)
            data = int(data_text, 16)
            is_write = kind == "W"
            selected = address in TX_WINDOW
            cs_n = 0 if selected else 1
            if negative and selected and is_write and not hidden and len(expected_bytes) == 17:
                cs_n = 1  # the RTL never sees this byte; the C model still expects it
                hidden = True
            rw = 0 if is_write else 1
            stimulus.append((cs_n << 9) | (rw << 8) | data)
            if selected and is_write:
                expected_bytes.append(data)
                current.append(data)
            if is_write and address == MB_CONTROL and data == CONTROL_SUBMIT:
                frames.append(current)
                current = []

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
         str(root / "rtl/atarix/atx_northbridge_bus_shim.v")],
        capture_output=True, text=True)
    if build.returncode != 0:
        print(build.stdout + build.stderr, file=sys.stderr)
        return 2

    run = subprocess.run(
        ["vvp", "-n", str(binary), f"+stimulus={stim_file}", f"+output={capture_file}",
         f"+cycles={len(stimulus)}"],
        capture_output=True, text=True, timeout=600)
    print(run.stdout.strip())
    if run.returncode != 0 or "FAIL" in run.stdout:
        print(run.stderr, file=sys.stderr)
        return 2

    captured = []
    sof_count = 0
    for line in capture_file.read_text().splitlines():
        value, sof = line.split()
        captured.append(int(value, 16))
        sof_count += int(sof)

    bytes_match = captured == expected_bytes
    frames_match = sof_count == len(frames)
    report = {
        "cpu_cycles": len(stimulus),
        "c_model_tx_bytes": len(expected_bytes),
        "rtl_captured_bytes": len(captured),
        "bytes_match": bytes_match,
        "c_model_frames": len(frames),
        "rtl_sof_markers": sof_count,
        "frames_match": frames_match,
        "frame_rule_strict": strict_frames,
    }
    if not bytes_match:
        for index, (got, want) in enumerate(zip(captured, expected_bytes)):
            if got != want:
                report["first_byte_mismatch"] = {"index": index, "rtl": got, "c_model": want}
                break
    (work / "bus_shim_cosim_report.json").write_text(json.dumps(report, indent=2) + "\n")

    print(f"bytes: RTL {len(captured)} vs C model {len(expected_bytes)} -> "
          f"{'MATCH' if bytes_match else 'MISMATCH'}")
    print(f"frames: C model {len(frames)} SUBMITs vs RTL {sof_count} SOF markers -> "
          f"{'MATCH' if frames_match else 'MISMATCH (known finding: shim framing rule unspecified)'}")

    if negative:
        if bytes_match:
            print("negative control FAILED: hidden byte was not detected", file=sys.stderr)
            return 1
        print("negative control passed: hidden byte detected as a byte-stream mismatch")
        return 0
    if not bytes_match:
        return 1
    if strict_frames and not frames_match:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
