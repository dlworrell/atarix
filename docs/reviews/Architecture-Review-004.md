---
document: Architecture-Review-004
title: "Architecture Review 004: Specification Gaps Found by the CPU Card Test Bench"
type: Architecture Review
status: Open
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-005-Mailbox-Model
  - ATX-SPEC-010-Audit-Model
  - ATX-SPEC-016-Supervisor-Management-Fabric
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:7ac01949-0789-402f-b361-ec42f87f0ace
-->

# Architecture Review 004: Specification Gaps Found by the CPU Card Test Bench

## Status

Open. Seven entries are resolved and need your confirmation; the others need
a decision or a specification change.

## Purpose

The [CPU card software test bench](../verification/cpu-card-software-testbench.md)
turns the ATARIX specifications into running code: a W65C816 test ROM, the
fabric Northbridge, the supervisor, and the production `libatarix` checks.
Wherever a specification is silent, contradicts another document, or names
something without encoding it, the bench has to choose. This register records
every such choice so you can accept it, change it, or write it into the
specification.

Read this if you own a specification below, or before you change ring,
mailbox, supervisor, discovery or audit behavior.

## How to read the register

Each gap has an ID, the documents involved, what is missing, what the code
does now, and the decision it needs.

| Status | Meaning |
|---|---|
| Resolved | Fixed in documents and code by this review. Confirm or revert. |
| Interim | The bench uses a profile of its own. The specification must adopt or replace it. |
| Open | Not implemented. A decision is needed before anyone can implement it. |
| Coverage | The specification is clear; the bench does not test it yet. |

## Decisions needed first

These five block the most work or carry the most security weight.

1. **G-RING-3: requests toward more privileged rings.** Today no ring can
   send a request to a more privileged ring, so an application cannot use any
   service. The model needs a mediated inward path.
2. **G-SUP-1 / G-SUP-2: the supervisor register interface.** Adopt the
   gateway profile (result codes, RESULT register, writable arguments) into
   `register-map-v1.md`, or replace it.
3. **G-SUP-4 / G-SUP-5: who may request recovery and reset,** and whether the
   kernel's reset request is DENIED or REQUIRES_APPROVAL.
4. **G-BOOT-1: the Discovery Services register block.** The documented block
   cannot express service binding by type.
5. **G-MBX-3: whether a rejected frame consumes its sequence number.**

## Rings

| ID | Documents | Gap | Code now | Status and decision |
|---|---|---|---|---|
| G-RING-1 | ADR-RING-SECURITY-MODEL, `security.md`, `core-doctrines.md`, `ring-security-test-plan.md`, `capability-model.md` | Three ring numberings were in use. The ADR has supervisor −2, fabric −1, kernel 0, driver 1, user service 2, application 3. `security.md` had a different ring 1 and ring 2. `core-doctrines.md` and the test plan used the seed model (supervisor 0, fabric 1, device 2, application 3, quarantine 4). `capability-model.md` called ring 2 a driver ring. | `rings.h` follows the ADR. | **Resolved.** The ADR is canonical and now has a full ring table. The other documents are aligned, with an old-to-new mapping where the seed model appeared. Confirm. |
| G-RING-2 | ADR-RING-SECURITY-MODEL, `capability-record-format-v1.md` | Code had ring 4 (quarantine) and its wire value `0x0004`; neither document defined it. | Quarantine requester denied everything; only the supervisor may act on a quarantined target. | **Resolved.** Defined as a containment state, not an execution ring; no capability may name it. |
| G-RING-3 | `ring.c`, `security.md`, `ring-security-test-plan.md`, ATX-SPEC-005 "Ring Boundaries" | `atarix_ring_can_access()` lets a ring request only its own or a less privileged ring. An application (3) cannot request a user service (2), and a user service cannot request a driver (1). `security.md` requires ring 3 to use services through mailboxes, and the test plan expected "Ring 3 → Ring 2 permitted". | The bench works only because its requester is the kernel (0), which requests outward. | **Open, high.** Decide how a less privileged ring requests a service in a more privileged ring: for example, allow REQUEST inward only to a declared service endpoint with a capability, and never MANAGE, QUARANTINE or RECOVER inward. Changing it changes `ring.c` and its tests. |
| G-RING-4 | `rings.h` | `atarix_ring_to_wire()` mapped any invalid ring to APPLICATION. | Was exploitable: a capability for kernel..application admitted a requester with ring 5 or −7. | **Resolved.** Invalid rings map to `0x8000`, which matches no range; the evaluator and policy also refuse invalid and quarantined requesters. New test `test_ring_wire_fail_closed` fails against the old code. |
| G-RING-5 | ADR-RING-SECURITY-MODEL, ADR-0016 | No rule said when a service belongs in ring 1 or ring 2. The GPU service sat in ring 2 as "service". | GPU compute moved to ring 1. | **Resolved.** The ADR now says: ring 1 if it operates hardware or holds device, DMA or interrupt capabilities; ring 2 otherwise. |

## Mailbox

| ID | Documents | Gap | Code now | Status and decision |
|---|---|---|---|---|
| G-MBX-1 | ATX-SPEC-005 "Error Handling", `mailbox.h` | The specification requires a "target unavailable" error; the v1 status set had none. | `ATARIX_MAILBOX_STATUS_TARGET_UNAVAILABLE` = 15. | **Resolved (interim value).** Confirm the value; it is ABI. |
| G-MBX-2 | ATX-SPEC-005 "Message Validation", `mailbox-protocol-v1.md` | The protocol's validation order stopped at capability, then dispatch. ATX-SPEC-005 also requires lifecycle state and resource availability. | Order continues: ring, capability, lifecycle, resources, dispatch. | **Resolved.** Protocol document updated. |
| G-MBX-3 | `mailbox-protocol-v1.md` "Sequence Rules" | Nothing says whether a frame rejected after the sequence check (for example by ring, capability, lifecycle or BUSY) consumes its sequence number. | It does. A sender retrying after BUSY must use a new sequence number. | **Interim.** Decide; the alternative is to commit the sequence only when the frame is dispatched. |
| G-MBX-4 | `mailbox-protocol-v1.md`, ATX-SPEC-005 | No status for "no such endpoint". | Unknown destination returns POLICY_DENIED, which does not reveal whether a node exists. | **Interim.** Confirm. |
| G-MBX-5 | ATX-SPEC-005 "Lifecycle" | Endpoint states are named but not encoded. | `atarix_mailbox_endpoint_state_t` 0–7 in the specification's listing order; only ACTIVE admits. | **Interim.** Confirm the values. |
| G-MBX-6 | ATX-SPEC-005 "Error Handling", "Versioning" | "Unsupported version" must be distinguishable, but the v1 header has no version field (the magic implies it) and there is no status value. | Not implemented. | **Open.** |
| G-MBX-7 | `mailbox-protocol-v1.md` | No rule maps a message type to the operation a capability must grant. | Queries and GPU transforms need READ, writes need WRITE, reset needs RESET, anything else CONTROL (`Northbridge::operation_for`). | **Interim.** |
| G-MBX-8 | ATX-SPEC-005 "Resource Accounting" | Queue depth, credits and reply slots are not defined. | One reply slot per CPU card; an unacknowledged reply makes the next request BUSY. | **Interim.** |

## Supervisor

| ID | Documents | Gap | Code now | Status and decision |
|---|---|---|---|---|
| G-SUP-1 | `register-map-v1.md` "Supervisor Mailbox", ATX-SPEC-016 "Operational Requests" | The register map lets the CPU write Reset Request, Recovery Mode and FPGA Reload as commands. ATX-SPEC-016 says a request is not a command and must pass a gateway and policy. (Test bench finding 6; closure unit ATX-SUP-REG-001 was DIVERGENT.) | `$00C200` is a Supervisor Request Gateway. Every request gets a policy decision and a result code, and is audited. | **Resolved in the bench.** `register-map-v1.md` must adopt the profile: RESULT at +06, ARG0–ARG3 read/write, interrupt on every completion. |
| G-SUP-2 | ATX-SPEC-016 "Request Results", "Command Model" | Result codes are named but not numbered. Command IDs exist only for 01–05. The command model requires a command version, but the register interface has nowhere to carry one. | Results 0 EXECUTED, 1 DENIED, 2 DEFERRED, 3 REQUIRES_APPROVAL, 4 QUARANTINED, 5 UNSUPPORTED, 6 UNKNOWN, 7 FAILED. Command 06 Report Fault is proposed. No version. | **Interim.** Confirm values and command 06; decide how the version is carried. |
| G-SUP-3 | ATX-SPEC-016 "Command Model" | Each command must define authority, policy, caller class, expected state, result, audit and failure behavior. None is defined. | Bench policy for a kernel caller: 01 Reset → REQUIRES_APPROVAL; 02 Recovery → DENIED; 03 Read Board ID → EXECUTED; 04 Read Fault Log → UNSUPPORTED; 05 FPGA Reload → DENIED; 06 Report Fault → EXECUTED for a less privileged endpoint (quarantined), otherwise DENIED; anything else → UNKNOWN. | **Interim.** Write the command table into ATX-SPEC-016. |
| G-SUP-4 | ATX-SPEC-016 "Required Tests" | "Recovery request enters recovery state" is required, but no caller class is named. | Recovery is denied to the kernel; no recovery state exists in the bench. | **Open.** Who may request recovery? |
| G-SUP-5 | ATX-SPEC-016 "Required Tests", "Reset And Power Model" | The required test says a reset request without authority is **denied**; the reset model says it **may require approval**. | REQUIRES_APPROVAL; nothing is reset. | **Open.** Choose one for a kernel caller. |
| G-SUP-6 | `register-map-v1.md` command 04 | The fault log record format is not defined. | UNSUPPORTED. | **Open.** |
| G-SUP-7 | ATX-SPEC-016 | Nothing says how the gateway learns who is calling on a register path. | From the slot, as for mailbox source identity. | **Interim.** |

## Enumeration, identity and boot

| ID | Documents | Gap | Code now | Status and decision |
|---|---|---|---|---|
| G-BOOT-1 | `register-map-v1.md` "Discovery Services" | The block has Device Count, Selected Device, Vendor ID, Device ID and Capability Pointer. Firmware cannot bind a service by type: there is no service type, node, ring, lifecycle state or instance. "Capability Pointer" is not defined. | Profile: +06 capability id granted to the reader (0 none), +07 ring, +08/+09 service type, +0A/+0B node, +0C lifecycle state, +0D instance. The CPU can write only SELECT. | **Interim.** |
| G-BOOT-2 | `identity-eeprom-v1.md`, `register-map-v1.md` | Identity vendor and device IDs are 32 bits; the Discovery registers are 16 bits. | Low 16 bits shown. | **Interim.** |
| G-BOOT-3 | `identity-eeprom-v1.md`, ADR-RING "Homebrew Cards" | Nothing says whether a card that fails identity validation is enumerated. | Enumerated and listed as QUARANTINED, with no capability. | **Interim.** Decide. Hiding it is the alternative. |
| G-BOOT-4 | `identity-eeprom-v1.md`, `capability.h` | No mapping from an identity's trust class and vendor to a trust level. | Homebrew vendor or HOMEBREW/UNKNOWN trust class → IDENTIFIED; otherwise VALIDATED. | **Interim.** |
| G-BOOT-5 | ADR-RING "Boot-Time Authority Chain", ATX-SPEC-013 | No format or content for the boot capability policy: which services the kernel receives, and what provider trust each needs. | READ on VALIDATION services needs IDENTIFIED; READ on GPU compute needs VALIDATED; nothing else is granted. | **Interim.** |
| G-BOOT-6 | `identity_eeprom.h` | No board class for display or GPU cards. | PROTOTYPE_CARD. | **Open.** Add a class. |
| G-BOOT-7 | ATX-SPEC-016 "Supervisor Audit Events" | No events for the boot chain's enumeration, capability evaluation, binding or transport activation steps, or for an identity failure. | `identity-validated`, `identity-failed`, `enumeration-complete`, `capability-granted`, `capability-withheld`, `service-binding`, `transport-activated`, `reset-released`. | **Interim.** |

## Audit

| ID | Documents | Gap | Code now | Status and decision |
|---|---|---|---|---|
| G-AUD-1 | ATX-SPEC-010 "Audit Record Requirements", ATX-SPEC-019 | Required fields include record ID, record version, timestamp source and an actor identity label. ATX-SPEC-019 says a location is not an identity, but no identity label is defined, so the bench can only record node IDs. | Each mailbox record has actor node and ring, target node and ring, operation, authority used (capability ID), policy result, result, reason, cycle and deciding stage. No record ID or version. | **Interim, partial.** Define the identity label and the record header. |
| G-AUD-2 | ATX-SPEC-010 "Additional metadata" | Binding ID has no object to refer to: binding is a directory publication, not an object with an ID. | Not recorded. | **Open.** |

## ATX-SPEC-016 required tests: coverage

| Required test | Bench |
|---|---|
| Operational request is not direct command | T26: Reset Request answered REQUIRES_APPROVAL; host checks nothing was reset |
| Unauthorized supervisor command denied | T41 Recovery, T43 FPGA Reload, T45 Report Fault on the supervisor |
| Supervisor request audit generated | Host checks every request-received and result event |
| Watchdog event audit generated | **Not covered:** no watchdog model |
| Reset request denied without authority | T26, but see G-SUP-5 |
| Recovery request enters recovery state | **Not covered:** see G-SUP-4 |
| Supervisor audit export is observational only | **Not covered:** no export path (G-SUP-6) |
| Operational fabric cannot mutate supervisor audit buffer | No CPU path reaches it; **no explicit test** |
| Unknown supervisor protocol version fails closed | **Not covered:** see G-SUP-2 |
| Supervisor authority does not become runtime authority | Partial: Report Fault quarantines but grants nothing |

Results DEFERRED, QUARANTINED and FAILED are defined but not exercised.

## Earlier findings still open

From the [test bench](../verification/cpu-card-software-testbench.md#findings):
finding 2 (headers not C++-clean), 5 (interrupt controller layout conflict),
7 (emulation vectors overlap native IRQ levels), 8 (audit window built from
registers) and 9 (ATX-SPEC-020 index messages rejected by the validator).

## Changes made by this review

Documents:

- ADR-RING-SECURITY-MODEL: ring table (all seven values), ring 4 section,
  ring 1 / ring 2 rule, amendment record.
- `security.md`, `core-doctrines.md`, `ring-security-test-plan.md`,
  `capability-model.md`, `service-directory-format-v1.md`,
  `capability-record-format-v1.md`, ADR-0016: aligned to the ADR.
- `mailbox-protocol-v1.md`: full validation order and failure statuses.
- `register-map-v1.md`, ATX-SPEC-005, ATX-SPEC-016: pointers to the open
  items here.
- The test bench document and its closure units.

Code: see the commits that introduce this review (`rings.h`, capability
evaluator and policy, `mailbox.h`, identity validation, the bench's
supervisor, Northbridge and test ROM).
