---
document: ATX-RTL-SHIM-001
title: Bus shim RTL against real CPU bus traffic
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
  - ATX-SPEC-020-Hardware-Mailbox-Integration-Notes
implements:
  - rtl/atarix/atx_northbridge_bus_shim.v
verified_by:
  - sim/rtl-cosim/run_bus_shim_cosim.py
  - scripts/rtl/run_shim_formal.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:0f99ba7b-9ff1-4756-9f16-ce3ff2330d37
-->

# ATX-RTL-SHIM-001: Bus shim RTL against real CPU bus traffic

```yaml
closure_unit:
  id: ATX-RTL-SHIM-001
  claim: >-
    rtl/atarix/atx_northbridge_bus_shim.v, driven by the emulated W65C816's
    recorded bus cycles, captures every mailbox TX-window write byte in order
    and marks one start-of-frame per mailbox frame.
  intent:
    authority: draft
    sources:
      - rtl/atarix/atx_northbridge_bus_shim.v (module comment)
      - docs/architecture/ATX-SPEC-020-Hardware-Mailbox-Integration-Notes.md
  realizations:
    rtl: required
    cosim_harness: required            # sim/rtl-cosim (Icarus)
    formal_harness: required           # sim/formal/bus_shim_formal.sv
    physical_hardware: deferred
  evidence:
    byte_stream_equivalence: required  # E4: 242/242 bytes match the C model (functional timing)
    frame_boundary_equivalence: required  # DIVERGENT: SOF per chip-select burst, not per frame
    datasheet_timing_capture: required    # 242/242 correct at 4 MHz PHI2 / 25 MHz and 14 MHz / 62.5 MHz (gating)
    formal_capture_proof: required        # E3+: proven for f_clk >= 4 x f_PHI2; lost/duplicate/corrupt impossible
    cosim_negative_control: required   # hidden byte detected; 14 MHz on 25 MHz rejected
    formal_negative_controls: required # ratio 3 steps, latency 5 steps, live-data mutant: counterexamples
  security_impact: none directly; framing errors would corrupt mailbox frames upstream of validation
  compatibility_impact: none
  recovery_impact: none
  state: DIVERGENT
```

## Divergence

The shim starts a frame on the first write after chip select deasserts. On a
real W65C816, instruction fetches separate almost every store, so chip select
drops between mailbox writes. The shim marks 140 SOF for 10 frames. The C
model frames on the CONTROL submit write.

### Timing (resolved)

The original shim checked chip select and captured data on the first
system-clock edge after it saw PHI2 low, up to 40 ns after the fall at 25 MHz.
The W65C816 holds write data and the address for about 10 ns. It captured 118
of 242 writes, only 15 correctly, and it fails the formal properties at every
clock ratio.

The shim now registers data, chip select and R/W on the PHI2 falling edge and
crosses to the system clock with a toggle and a two-flop synchronizer. It is
proven for f_clk ≥ 4 × f_PHI2 (each PHI2 phase at least two system clocks),
with delivery within three system clocks. Remaining hardware conditions:

- PHI2 must reach a clock-capable FPGA pin;
- 8 and 14 MHz operation needs an ECP5 PLL above the 25 MHz oscillator.

### Framing

Engineering decision needed: specify the framing rule, for example frame on
CONTROL submit, a length-driven frame from the header, or an explicit
start-of-frame register. Then change the RTL and run the co-simulation with
`--expect-sof-per-frame`.
