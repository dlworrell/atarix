<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:58ac45fc-3a1d-4ebf-8fb3-cf4dde095d6b
-->

# ATARIX Core Doctrines

Status: Baseline

## Purpose

This document records the non-negotiable architectural doctrines that guide ATARIX design, implementation, simulation, testing, and hardware validation.

These doctrines are intentionally short. They are review rules, not implementation details.

## Doctrines

### 1. No Direct Hardware Access

Software components do not directly access protected hardware, registers, buses, slots, or physical addresses.

All protected access occurs through services, capabilities, resources, and implementation layers.

### 2. Discovery Grants Visibility

Discovery reveals that resources or services exist.

Discovery does not grant authority to use them.

### 3. Capabilities Grant Authority

Capabilities are the authorization mechanism for protected operations.

A request without a valid capability is denied.

### 4. Deny By Default

The default answer to every protected operation is DENY.

Access is allowed only after all applicable validation, trust, ring, ownership, delegation, revocation, expiration, resource, and operation checks pass.

### 5. Hardware Is An Implementation Detail

Software targets services, not hardware placement.

A service may be implemented in FPGA logic, a CPU card, a peripheral card, a simulator, or a future node without changing the public contract.

### 6. Failures Must Be Observable

Failures must be detected, classified, reported, and traceable.

Silent failure is not acceptable.

### 7. Transport Independence

Services must not depend on a specific transport unless the transport itself is the service contract.

Mailbox, local calls, simulated links, FPGA paths, and future inter-node transports should preserve the same semantics.

### 8. Revocation Must Be Immediate

A revoked capability ceases to authorize operations immediately.

Revocation overrides ownership, trust, and convenience.

### 9. Least Privilege

Capabilities and services grant only the authority required for the requested function.

Delegation may reduce authority only.

### 10. Security Before Convenience

Architectural security takes precedence over shortcuts, performance hacks, simulator convenience, or direct-access workarounds.

### 11. No Address-Based Authority

Knowledge of an address, slot, register, mailbox endpoint, handle, service name, or transport identifier grants zero authority.

Authority is conveyed only by valid capabilities evaluated by policy.

### 12. Simulator Equals Hardware

The simulator is a reference implementation, not a shortcut or mock that bypasses security.

Simulator security rules, capability checks, mailbox validation, and service boundaries must match the future hardware model.

### 13. Ring Security Is Mandatory

ATARIX uses ring-based security boundaries.

Higher-trust rings may supervise, delegate, constrain, or revoke authority from lower-trust rings.

Lower-trust rings may not directly command, inspect, mutate, bypass, or impersonate higher-trust rings.

Ring boundaries are enforced by Supervisor policy, capability checks, mailbox validation, service dispatch, and fabric routing rules.

Capabilities grant authority only within the maximum authority permitted by the requester's ring and the target's ring policy.

## Required Access Path

The permitted protected-access path is:

```text
Application
    -> Service
    -> Capability Policy
    -> Ring Policy
    -> Resource
    -> Implementation
```

The forbidden path is:

```text
Application
    -> Address
    -> Hardware
```

## Security Decision Chain

Protected operations must satisfy the full security chain:

```text
Deny by default
    -> Ring boundary check
    -> Capability validation
    -> Revocation check
    -> Mailbox validation
    -> Service policy
    -> Resource policy
    -> Authorized operation
```

Failure at any step denies the operation.

## Ring Model

The existence of ring boundaries is mandatory. The rings themselves are
defined by [ADR-RING-SECURITY-MODEL](ADR-RING-SECURITY-MODEL.md), which
supersedes the seed model this doctrine first suggested:

```text
Ring -2: Supervisor (platform)
Ring -1: Fabric / platform firmware (platform)
Ring  0: Kernel / executive
Ring  1: Drivers and hardware-facing services
Ring  2: User services
Ring  3: Applications
Ring  4: Quarantine (containment state, not an execution ring)
```

The seed model numbered the rings differently. Read older documents with
this mapping:

| Earlier seed number | Earlier name | Canonical ring (ADR) |
|---|---|---|
| Ring 0 | Supervisor / Root Authority | Ring −2 Supervisor |
| Ring 1 | Fabric Management / Trusted System Services | Ring −1 Fabric (system services move to Ring 0–2 by role) |
| Ring 2 | Device Services / Card Firmware | Ring 1 Driver |
| Ring 3 | Applications / User Workloads | Ring 3 Application |
| Ring 4 | Untrusted or Quarantined Nodes | Ring 4 Quarantine (containment state) |

## Review Rule

Every significant ATARIX change should answer:

```text
Which doctrine does this preserve?
Which ADR justifies this behavior?
Which public API expresses the contract?
Which test or bench procedure enforces it?
```

If the answer is unclear, the design is not ready.

## Result

These doctrines define the ATARIX architectural baseline.

Future subsystems may extend the system, but they must not weaken these rules without an explicit ADR.
