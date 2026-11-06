---
document: ATX-CPU-BOOT-001
title: CPU card reset to native mode on the Rev A map
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
implements:
  - sim/cpu-card/cpu_card.cpp
  - sim/cpu-card/firmware/test_rom.s
verified_by:
  - scripts/sim/run_testbench.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:b4b6c5c9-49b9-4e5f-939f-9e3605b872c5
-->

# ATX-CPU-BOOT-001: CPU card reset to native mode on the Rev A map

```yaml
closure_unit:
  id: ATX-CPU-BOOT-001
  claim: >-
    From power-on the CPU card fetches the reset vector from ROM, enters native
    mode, and sees RAM, ROM, UART and board identity at the Rev A addresses.
    ROM is write-protected and every unmapped access is an observable fault.
  intent:
    authority: draft
    sources:
      - docs/memory-map.md
      - docs/register-map-v1.md
      - docs/boot-sequence-v1.md
  realizations:
    emulator_card: required            # sim/cpu-card/cpu_card.cpp
    test_firmware: required            # sim/cpu-card/firmware/test_rom.s
    monitor_firmware: deferred         # src/firmware/monitor is a README only
    rtl_address_decode: deferred
    physical_hardware: deferred
  evidence:
    emulator_boot_run: required        # E2/E6-in-simulation: PASS, 31,000 cycles
    fault_accounting: required         # exactly one fault: the deliberate ROM write
    hardware_capture: deferred
  security_impact: unmapped access and ROM writes are faulted, never silent
  compatibility_impact: none
  recovery_impact: none
  state: VERIFYING
```

## Evidence

`scripts/sim/run_testbench.sh`: the test ROM reaches native mode (verified with
`CLC; XCE` returning carry clear). It checks RAM in bank `$00` and through
long addressing in banks `$01` and `$7F`, and confirms a ROM write is ignored.
It reads board identity `A7A1/0816`, then reaches STP. The host accepts exactly
one fault, the ROM write at `$00E000`.

## Gaps

- The memory map is "first-pass" in `memory-map.md`; the expansion window and
  banks above `$7F` are modeled as unmapped.
- The board identity values are simulator values, not registered identifiers.
- No interrupt, ABORT or watchdog path is exercised yet.
