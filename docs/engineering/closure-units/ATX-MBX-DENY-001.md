---
document: ATX-MBX-DENY-001
title: CPU mailbox requests fail closed and are audited
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
  - ATX-SPEC-003-Capability-Model
  - ATX-SPEC-005-Mailbox-Model
implements:
  - sim/cpu-card/northbridge.cpp
verified_by:
  - sim/cpu-card/negative_control.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:4f189244-815d-41d2-a180-3223a1e7f978
-->

# ATX-MBX-DENY-001: CPU mailbox requests fail closed and are audited

```yaml
closure_unit:
  id: ATX-MBX-DENY-001
  claim: >-
    A mailbox request from the CPU card is rejected, with an explicit status
    and an audit record, when its CRC is wrong, its sequence is replayed or
    gapped, its type is unknown, its endpoint is unknown, its ring may not
    request the target ring, or it lacks a capability for the operation; and,
    once authorized, when the target endpoint is not ACTIVE
    (TARGET_UNAVAILABLE), is quarantined (QUARANTINED), or has no free reply
    slot (BUSY). Each audit record names the authority (capability) used.
    The CPU cannot choose its own source identity.
  intent:
    authority: normative
    sources:
      - docs/mailbox-protocol-v1.md
      - docs/doctrine/ARC-SEC.md
      - docs/architecture/ATX-SPEC-003-Capability-Model.md
      - docs/architecture/ATX-SPEC-005-Mailbox-Model.md   # lifecycle and resource checks
      - docs/architecture/ATX-SPEC-010-Audit-Model.md     # audit record fields
  realizations:
    emulator: required                 # Northbridge model, protocol validation order
    firmware: required                 # test ROM cases T12-T1A, T35-T38, T47
    rtl: deferred                      # no mailbox-engine RTL
    physical_hardware: deferred
  evidence:
    emulator_tests: required           # all 21 decisions and their authority match the host's expected audit
    independent_audit_check: required  # host compares the Northbridge audit log, not firmware output
    negative_controls: required        # skip-capability, skip-ring, accept-bad-crc, corrupt-pong, skip-lifecycle, skip-resource, liar ROM: all detected
    hardware_capture: deferred
  security_impact: >-
    ring and capability enforcement at the Northbridge; source identity is
    assigned from the slot, never from the frame
  compatibility_impact: none
  recovery_impact: none
  state: VERIFYING
```

## Open decisions

- The message-type-to-operation mapping (`Northbridge::operation_for`).
- Unknown endpoint reported as `POLICY_DENIED` (G-MBX-4 in Architecture Review 004).
- Whether a rejected frame consumes its sequence number (G-MBX-3).
- Audit record ID, version and identity labels are not defined (G-AUD-1).
- Revocation and time-limited capabilities are evaluated by `capability_policy.c`,
  which this path does not call yet: the Northbridge uses `atarix_capability_evaluate`.
