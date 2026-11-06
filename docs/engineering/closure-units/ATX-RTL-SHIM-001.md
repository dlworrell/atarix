---
document: ATX-RTL-SHIM-001
title: Bus shim and TX frame assembler RTL against real CPU bus traffic
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
  - ATX-SPEC-020-Hardware-Mailbox-Integration-Notes
implements:
  - rtl/atarix/atx_northbridge_bus_shim.v
  - rtl/atarix/atx_mailbox_tx_assembler.v
verified_by:
  - sim/rtl-cosim/run_bus_shim_cosim.py
  - scripts/rtl/run_shim_formal.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:0f99ba7b-9ff1-4756-9f16-ce3ff2330d37
-->

# ATX-RTL-SHIM-001: Bus shim and TX frame assembler RTL against real CPU bus traffic

```yaml
closure_unit:
  id: ATX-RTL-SHIM-001
  claim: >-
    rtl/atarix/atx_northbridge_bus_shim.v and atx_mailbox_tx_assembler.v,
    driven by the emulated W65C816's recorded bus cycles at datasheet timing,
    deliver every write to the mailbox block (address and data) once and in
    order, and emit exactly the frames the C Northbridge validates, one per
    SUBMIT.
  intent:
    authority: draft
    sources:
      - docs/verification/cpu-card-software-testbench.md (Mailbox Engine register profile)
      - docs/architecture/ATX-SPEC-020-Hardware-Mailbox-Integration-Notes.md
  realizations:
    rtl: required
    cosim_harness: required            # sim/rtl-cosim (Icarus)
    formal_harness: required           # sim/formal/bus_shim_formal.sv
    physical_hardware: deferred
  evidence:
    write_equivalence: required        # E4: all mailbox-block writes match the C model
    frame_equivalence: required        # E4: all 16 frames match; type and sequence match the C audit
    datasheet_timing: required         # 4 MHz/25 MHz and 14 MHz/62.5 MHz: writes and frames match (gating)
    formal_capture_proof: required     # E3+: shim proven for f_clk >= 4 x f_PHI2 (address and data)
    cosim_negative_controls: required  # hidden write, hidden SUBMIT, 14 MHz on 25 MHz: all detected
    formal_negative_controls: required # ratio 3 steps, latency 5 steps, live-data mutant: counterexamples
    hardware_capture: deferred
  security_impact: frames reach mailbox validation intact; nothing here grants authority
  compatibility_impact: shim interface changed (addr_in, shim_addr_out added; shim_sof removed)
  recovery_impact: frame_overrun latches if a SUBMIT arrives while a frame is streaming
  state: VERIFYING
```

## Timing (resolved)

The original shim checked chip select and captured data on the first
system-clock edge after it saw PHI2 low, up to 40 ns after the fall at 25 MHz.
The W65C816 holds write data and the address for about 10 ns. It captured 118
of 242 writes, only 15 correctly, and fails the formal properties at every
clock ratio.

The shim now registers address, data, chip select and R/W on the PHI2 falling
edge, then crosses to the system clock with a toggle and a two-flop
synchronizer. It is proven for f_clk ≥ 4 × f_PHI2, with delivery within three
system clocks.

## Framing (resolved)

A byte stream cannot carry framing, because firmware may write the TX window
in any order. The scaffold's start-of-frame rule marked 154 starts for 11
frames. The frame is now defined by the register interface:

- the assembler keeps the 64-byte window;
- on a SUBMIT write to CONTROL it copies the window;
- it streams the 16-byte header plus the claimed payload (capped at 48 bytes).

This is the C model's rule, so RTL and C are compared frame by frame.

## Remaining gaps

- PHI2 must reach a clock-capable FPGA pin. 8 and 14 MHz operation needs an
  ECP5 PLL above the 25 MHz oscillator.
- The assembler is checked by co-simulation, not yet by a formal proof.
- No mailbox validation or reply path exists in RTL yet: the C Northbridge
  still makes every decision.
