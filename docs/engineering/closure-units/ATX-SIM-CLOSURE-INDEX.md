---
document: ATX-SIM-CLOSURE-INDEX
title: Closure Units: CPU Card Software Test Bench
type: Engineering Record
status: Draft
lifecycle: Draft
related:
  - closure-development-cycle
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:9b8b34b4-5d42-4185-b075-c4ef5f0ca93b
-->

# Closure Units: CPU Card Software Test Bench

Status: Draft
Owner: ATARIX
Authority: [Closure-Based Development Cycle](../closure-development-cycle.md)

Closure records for the behavior exercised by the
[CPU Card Software Test Bench](../../verification/cpu-card-software-testbench.md).
Evidence levels follow the ladder in the closure cycle (E0 structural to E6
system). None of these units can reach CLOSED for a physical claim with
emulator evidence alone; physical realizations are recorded as `deferred`,
which is an open gap, not "not applicable".

| Unit | Claim | State |
|---|---|---|
| [ATX-CPU-VEC-001](ATX-CPU-VEC-001.md) | The emulated CPU matches W65C816 instruction-level behavior | VERIFYING |
| [ATX-CPU-BOOT-001](ATX-CPU-BOOT-001.md) | The CPU card boots from the reset vector into native mode on the Rev A map | VERIFYING |
| [ATX-MBX-PING-001](ATX-MBX-PING-001.md) | A valid PING returns PONG and preserves the transaction identifier | VERIFYING |
| [ATX-MBX-DENY-001](ATX-MBX-DENY-001.md) | Invalid, unauthorized or undeliverable CPU mailbox requests are rejected and audited | VERIFYING |
| [ATX-RTL-SHIM-001](ATX-RTL-SHIM-001.md) | Bus shim and frame assembler RTL deliver the writes and frames the C model sees | VERIFYING |
| [ATX-IRQ-VPR-001](ATX-IRQ-VPR-001.md) | Prioritized IRQs reach per-level handlers through Vector Pull Rewrite | VERIFYING |
| [ATX-SUP-REG-001](ATX-SUP-REG-001.md) | The CPU requests supervisor actions only through the Supervisor Request Gateway | VERIFYING |
| [ATX-BOOT-CHAIN-001](ATX-BOOT-CHAIN-001.md) | The supervisor runs the boot authority chain before the CPU card leaves reset | VERIFYING |
| [ATX-FPGA-ECP5-001](ATX-FPGA-ECP5-001.md) | Northbridge RTL fits the ULX3S ECP5-85F at the board clock | VERIFYING |
| [ATX-GPU-SVC-001](ATX-GPU-SVC-001.md) | The CPU card computes transforms through the cglm-backed GPU service, by capability | VERIFYING |
