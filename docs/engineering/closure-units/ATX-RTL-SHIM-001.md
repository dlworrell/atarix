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
    byte_stream_equivalence: required  # E4: 220/220 bytes match the C model
    frame_boundary_equivalence: required  # DIVERGENT: 140 SOF markers vs 10 frames
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

Engineering decision needed: specify the framing rule, for example frame on
CONTROL submit, a length-driven frame from the header, or an explicit
start-of-frame register. Then change the RTL and run the co-simulation with
`--expect-sof-per-frame`.
