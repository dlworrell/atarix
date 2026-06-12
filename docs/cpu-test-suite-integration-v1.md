# CPU Test Suite Integration v1

## Purpose

This document defines how ATARIX should reuse external 65x02-family CPU test material for emulator validation, firmware validation, and eventual hardware bring-up.

The immediate source candidate is:

```text
dlworrell/65x02
```

That repository describes itself as a language-agnostic JSON-encoded instruction-by-instruction test suite for the 65[c]02 family that includes bus activity. Its README states that each test requires execution of only a single instruction and provides full processor and memory state before and after execution.

## Licensing

The `dlworrell/65x02` repository carries an MIT License.

Any copied or vendored test material must preserve the upstream copyright and license notice.

## Integration Policy

Do not blindly copy the entire upstream repository into ATARIX until the needed subset is identified.

Preferred approach:

1. Add a documented upstream reference.
2. Identify the exact CPU families and test sets needed.
3. Vendor only the test data required for ATARIX validation, or add a scripted fetch mechanism.
4. Preserve upstream license text.
5. Keep ATARIX-specific expected behavior and harness code separate from upstream data.

## Intended Uses

### Emulator Validation

The first use should be validating the ATARIX emulator CPU core or any CPU-core integration layer.

Use cases:

- single-instruction execution tests
- register before/after validation
- status flag validation
- memory before/after validation
- bus-cycle trace comparison where available

### Firmware Validation

The second use should be generating firmware-level validation ROMs or monitor commands that can run selected test vectors on ATARIX hardware.

These tests should not assume a full operating system.

### Hardware Bring-Up

The third use should be hardware-facing CPU validation.

These tests should help validate:

- reset behavior
- native-mode transition
- emulation-mode behavior
- address decoding
- interrupt routing
- bus timing assumptions
- watchdog interaction
- supervisor fault capture

## Required CPU Coverage

ATARIX requires validation coverage for:

```text
6502
65C02
W65C816
```

The `65x02` repository appears immediately useful for 6502 and 65C02-style instruction validation.

W65C816-specific behavior must be audited separately.

## Required ATARIX-Specific Coverage

The upstream test suite is instruction-level CPU validation. ATARIX also needs platform-level validation.

Missing ATARIX-specific coverage includes:

- W65C816 native mode
- W65C816 emulation mode transition
- 24-bit addressing
- bank register behavior
- direct page relocation
- stack relocation
- MVN and MVP block moves
- COP handling
- ABORT handling
- WAI behavior
- STP behavior
- IRQ routing through the Fabric Northbridge
- NMI routing
- reset vector behavior
- CPU-local SRAM decode
- CPU-local DMA buffer behavior
- mailbox access from CPU firmware
- Identity EEPROM read path
- supervisor watchdog interaction
- validation boot mode
- Backplane BIOS handoff
- network-first boot prerequisites

## Test Tree Plan

Create:

```text
tests/
├── cpu/
│   ├── upstream/
│   ├── harness/
│   └── atarix-specific/
├── eeprom/
├── discovery/
├── mailbox/
└── service_directory/
```

## Upstream Import TODO

- [ ] Inventory `dlworrell/65x02` repository tree.
- [ ] Identify available 6502 test sets.
- [ ] Identify available 65C02 test sets.
- [ ] Determine whether any W65C816 tests exist upstream.
- [ ] Preserve upstream MIT license in `tests/cpu/upstream/`.
- [ ] Decide between vendoring JSON test data or scripted download.
- [ ] Write ATARIX test manifest.
- [ ] Write emulator-side JSON runner.
- [ ] Write hardware validation ROM generator.
- [ ] Add CI test target for emulator CPU tests.

## Validation Strategy TODO

- [ ] Define pass/fail record format.
- [ ] Define failure reporting format for monitor output.
- [ ] Define supervisor event codes for CPU validation failure.
- [ ] Define validation boot-mode behavior.
- [ ] Define how test failures are exposed through the engineering console.
- [ ] Define reduced smoke-test set for hardware.
- [ ] Define full exhaustive emulator test set.

## Principle

Instruction-level CPU tests prove the CPU core.

ATARIX validation must also prove that the CPU is correctly integrated into the system.
