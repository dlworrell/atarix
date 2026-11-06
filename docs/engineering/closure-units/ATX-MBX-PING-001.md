---
document: ATX-MBX-PING-001
title: Mailbox PING from the CPU card
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
  - ATX-SPEC-005-Mailbox-Model
implements:
  - sim/cpu-card/northbridge.cpp
  - sim/cpu-card/firmware/test_rom.s
verified_by:
  - scripts/sim/run_testbench.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:48b0ff4b-761f-4b81-998b-7468e89432e8
-->

# ATX-MBX-PING-001: Mailbox PING from the CPU card

The example closure unit from the closure-development cycle, now with
realizations and evidence.

```yaml
closure_unit:
  id: ATX-MBX-PING-001
  claim: A valid mailbox PING returns PONG and preserves the transaction identifier.
  intent:
    authority: normative
    sources:
      - docs/mailbox-protocol-v1.md
  realizations:
    public_header: required            # include/atarix/mailbox.h (existing)
    firmware: required                 # test ROM builds frames and CRC32 on the 65816
    emulator: required                 # Northbridge model calling libatarix
    rtl: required                      # bus shim captures bytes; no mailbox-engine RTL yet
    physical_hardware: deferred
  evidence:
    encoding_vectors: required         # existing tests/mailbox (make check)
    emulator_tests: required           # T11 and T16: PONG, sequence, payload, CRC verified by firmware
    rtl_simulation: required           # partial: byte capture only (ATX-RTL-SHIM-001)
    differential_test: required        # firmware CRC32 == atarix_mailbox_crc32 on every frame
    hardware_capture: deferred
  security_impact: capability check (READ on the echo service) and audit record per request
  compatibility_impact: none
  recovery_impact: transaction identifier preserved in the audit record
  state: VERIFYING
```

## Evidence

- The 65816 firmware computes CRC32 (reflected, `0xEDB88320`). The production
  C validator accepts every valid frame, and the firmware recomputes and
  accepts every PONG CRC the C code built. That covers both directions.
- PONG checks: RX-ready, type, source node `0xFF00`, length, magic, header
  type, length, sequence equal to the request, reserved word zero, payload
  byte-for-byte, and that RX-ready clears after acknowledgment.

## Gaps

- No mailbox-engine RTL exists. The RTL step reaches the bus shim only, and
  its framing diverges (ATX-RTL-SHIM-001). This unit cannot close until RTL
  produces the PONG.
- The Mailbox Engine register profile is a proposal (see the test-bench doc).
