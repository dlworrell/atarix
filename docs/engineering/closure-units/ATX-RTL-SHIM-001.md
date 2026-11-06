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
    physical_hardware: deferred
  evidence:
    byte_stream_equivalence: required  # E4: 242/242 bytes match the C model (functional timing)
    frame_boundary_equivalence: required  # DIVERGENT: 154 SOF markers vs 11 frames
    datasheet_timing_capture: required    # DIVERGENT: 118 of 242 writes captured, 15 correct (25 MHz clock, 4 MHz PHI2)
    cosim_negative_control: required   # hidden byte detected
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

### Timing

The shim checks chip select and captures data on the first system-clock edge
after it sees PHI2 low, which can be up to 40 ns after the fall at 25 MHz. The
W65C816 holds write data and the address for about 10 ns. At datasheet timing,
the shim misses about half the writes and corrupts nearly all of the rest.
PHI2, CS and R/W also have no synchronizer. Fix the capture first:

- latch data and CS in I/O registers on PHI2;
- or oversample from a PLL clock and sample during PHI2 high.

Then run with `--datasheet-timing --expect-datasheet-timing`.

### Framing

Engineering decision needed: specify the framing rule, for example frame on
CONTROL submit, a length-driven frame from the header, or an explicit
start-of-frame register. Then change the RTL and run the co-simulation with
`--expect-sof-per-frame`.
