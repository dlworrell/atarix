---
document: ATX-IRQ-VPR-001
title: Prioritized IRQ dispatch through Vector Pull Rewrite
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
implements:
  - sim/cpu-card/interrupts.cpp
  - sim/cpu-card/firmware/test_rom.s
verified_by:
  - sim/cpu-card/negative_control.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:b8c21991-8153-442d-b369-2506ab7446e8
-->

# ATX-IRQ-VPR-001: Prioritized IRQ dispatch through Vector Pull Rewrite

```yaml
closure_unit:
  id: ATX-IRQ-VPR-001
  claim: >-
    Fabric interrupt sources are masked at boot and latched while masked.
    Once unmasked, the highest-priority source is delivered first, and Vector
    Pull Rewrite steers its IRQ vector fetch to that level's own handler
    ($FFEE + 2 x level). No native-mode IRQ is ever redirected to RESET.
  intent:
    authority: draft
    sources:
      - docs/interrupt-architecture-v1.md
      - docs/vega816-analysis.md
      - dlworrell/Vega816 README, "Vector Pull Rewrite Shim" (8dfb370)
      - docs/cpu-card-architecture-v1.md
  realizations:
    emulator: required                 # controller and rewrite on the CPU card bus (VPB from the CLK core)
    firmware: required                 # per-level native IRQ handlers in the test ROM
    rtl: deferred                      # no interrupt controller or rewrite RTL yet
    physical_hardware: deferred        # 74HC283 adder shim or FPGA logic
  evidence:
    emulator_tests: required           # T21-T25 pass
    vector_fetch_trace: required       # host checks $FFEE->$FFEE (level 0), then $FFEE->$FFF2 (level 2)
    negative_controls: required        # no-rewrite, ignore-mask, level-7 (jumps to RESET): all detected
    hardware_capture: deferred
  security_impact: >-
    interrupts notify only; the level selects a handler and grants nothing.
    Level 7 in native mode would re-enter RESET without a hardware reset, so it
    is never issued.
  compatibility_impact: native IRQ levels 3-6 share the emulation COP/ABORT/NMI vector slots
  recovery_impact: none
  state: VERIFYING
```

## Open decisions

- The level mapping from source to vector.
- Rewrite in native mode only.
- Which register layout is authoritative (`register-map-v1.md` and
  `interrupt-architecture-v1.md` disagree).
- Whether Vector Pull Rewrite is required for Rev A or reserved for Rev B
  (`cpu-card-architecture-v1.md` open question).
