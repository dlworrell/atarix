---
document: ATX-FPGA-ECP5-001
title: Northbridge RTL fits the ULX3S ECP5-85F at the board clock
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
implements:
  - rtl/atarix/atx_northbridge_bus_shim.v
  - rtl/atarix/atx_spec_020_system_wrapper.v
verified_by:
  - scripts/rtl/run_ecp5_fit.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:d03cfaf4-27a6-4304-bc7c-e4beff76cee2
-->

# ATX-FPGA-ECP5-001: Northbridge RTL fits the ULX3S ECP5-85F at the board clock

```yaml
closure_unit:
  id: ATX-FPGA-ECP5-001
  claim: >-
    The current Fabric Northbridge RTL synthesizes, places and routes on the
    LFE5U-85F-6BG381C (the ULX3S part selected in ADR-0001) and meets the
    ULX3S 25 MHz oscillator frequency.
  intent:
    authority: normative
    sources:
      - docs/adr/ADR-0001-ulx3s-selection.md
      - docs/ulx3s-backplane-controller.md
      - dlworrell/ulx3s 14cac31 (README Features; doc/constraints/ulx3s_v20.lpf clk_25mhz)
  realizations:
    rtl: required
    board_pinout: deferred             # out-of-context: no I/O placement decided
    bitstream: deferred
    physical_hardware: deferred
  evidence:
    synthesis_and_pnr: required        # E3: shim 432 MHz, 17 LUT; ATX-SPEC-020 wrapper 76.7 MHz, 726 LUT, 1 DSP
    negative_control: required         # unreachable 1000 MHz target rejected
    on_board_execution: deferred       # E5
  security_impact: none
  compatibility_impact: none
  recovery_impact: none
  state: VERIFYING
```

## Gaps

- Out-of-context placement only. The I/O timing that matters for the CPU bus
  (ATX-RTL-SHIM-001) needs a pinout and I/O constraints.
- No bitstream has been produced or loaded on a board.
