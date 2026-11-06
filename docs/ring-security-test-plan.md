<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:c0915fda-5ec8-403f-a333-e82b70a3701b
-->

# ATARIX Ring Security Test Plan

Status: Implemented (the tests below exist and run in `make check`). Ring
numbers updated 2026-11-06 to the canonical model.

## Purpose

This document records the test additions required to make ATARIX ring security executable rather than merely documented.

Existing tests should not be rewritten. The ring-security work adds new tests that prove ring policy is enforced before capability authority, mailbox delivery, fabric routing, and Supervisor service access.

## Security Model Under Test

ATARIX uses two related but separate protection mechanisms:

```text
Ring security       = coarse trust boundary
Capability security = fine-grained authority boundary
```

The ring boundary is the large-gauge net.

Capabilities are the fine mesh.

A capability may reduce, delegate, expire, or revoke authority, but it may not grant authority that violates ring policy.

Required decision chain:

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

## Baseline Ring Model

Rings are defined by [ADR-RING-SECURITY-MODEL](ADR-RING-SECURITY-MODEL.md)
and `include/atarix/rings.h`:

```text
Ring -2: Supervisor            Ring 1: Drivers and hardware-facing services
Ring -1: Fabric / platform     Ring 2: User services
Ring  0: Kernel / executive    Ring 3: Applications
Ring  4: Quarantine (containment state)
```

This plan was first written against the seed numbering. The expected
behaviors below are translated to the canonical rings:

| Earlier seed number | Earlier name | Canonical ring (ADR) |
|---|---|---|
| Ring 0 | Supervisor / Root Authority | Ring −2 Supervisor |
| Ring 1 | Fabric Management / Trusted System Services | Ring −1 Fabric (system services move to Ring 0–2 by role) |
| Ring 2 | Device Services / Card Firmware | Ring 1 Driver |
| Ring 3 | Applications / User Workloads | Ring 3 Application |
| Ring 4 | Untrusted or Quarantined Nodes | Ring 4 Quarantine (containment state) |

The existence of ring boundaries is mandatory.

## Current Tests That Remain Valid

The following existing test families remain valid and should not be rewritten simply because ring policy is being added:

```text
discovery/*
capability/*
mailbox/*
sim/*
```

Ring security adds new test coverage. It does not invalidate the current 29-test baseline.

## New Ring Manager Tests

Planned test files:

```text
tests/ring/test_ring_self_access.c
tests/ring/test_ring_upward_denied.c
tests/ring/test_ring_downward_allowed.c
tests/ring/test_ring_quarantine_denied.c
tests/ring/test_ring_supervisor_override.c
```

### test_ring_self_access

Proves that an actor may access resources within its own ring when operation policy allows it.

Expected behavior:

```text
Ring N -> Ring N: allowed for permitted operation classes
```

### test_ring_upward_denied

Proves lower-trust rings cannot directly command, inspect, mutate, bypass, or impersonate higher-trust rings.

Expected behavior:

```text
Ring 3 -> Ring -2: denied
Ring 1 -> Ring -2: denied unless through the Supervisor Request Gateway (ATX-SPEC-016)
Ring 4 -> any ring: denied
```

### test_ring_downward_allowed

Proves higher-trust rings may supervise, constrain, or revoke authority from lower-trust rings.

Expected behavior:

```text
Ring -2 -> Ring -1: allowed for Supervisor operations
Ring -2 -> Ring  1: allowed for Supervisor operations
Ring -1 -> Ring  1: allowed for Fabric-management operations
```

### test_ring_quarantine_denied

Proves quarantined nodes cannot reach trusted services or other normal nodes.

Expected behavior:

```text
Ring 4 -> Ring -2: denied
Ring 4 -> Ring -1: denied
Ring 4 -> Ring  1: denied
Ring 4 -> Ring  3: denied (the code has no quarantine-service exception)
```

### test_ring_supervisor_override

Proves Ring 0 can perform recovery, revocation, and quarantine actions that lower rings cannot perform.

Expected behavior:

```text
Ring -2 may quarantine less privileged rings.
Ring -2 may revoke less privileged authority.
Ring -2 may inspect Supervisor-owned tables.
Less privileged rings may not override Ring -2 policy.
```

## Capability/Ring Interaction Tests

Planned test files:

```text
tests/capability/test_capability_respects_ring_boundary.c
tests/capability/test_delegation_cannot_cross_ring_boundary.c
tests/capability/test_revoked_capability_still_denied_after_ring_check.c
```

### test_capability_respects_ring_boundary

Proves a syntactically valid capability is still denied if ring policy forbids the request.

Example:

```text
Ring 3 application holds a valid-looking capability for a Ring -2 (supervisor) endpoint.
Ring policy denies before resource access.
```

### test_delegation_cannot_cross_ring_boundary

Proves delegation cannot increase authority across a prohibited ring boundary.

Example:

```text
Ring 1 device service delegates to Ring 3 application.
Delegated capability may only reduce authority.
Delegation cannot grant Ring 3 direct access to Ring -1 or Ring -2 internals.
```

### test_revoked_capability_still_denied_after_ring_check

Proves revocation remains effective even when ring policy would otherwise allow the request.

Example:

```text
Ring 1 -> Ring 3 permitted by ring policy (the test uses driver -> application).

Note: the plan originally said "Ring 3 -> Ring 2 permitted by ring policy".
The code denies every request from a less privileged ring to a more
privileged one, so an application cannot request a user service by ring
policy at all. This is open gap G-RING-3 in Architecture Review 004.
Capability exists but is revoked.
Operation is denied.
```

## Mailbox/Ring Interaction Tests

Planned test files:

```text
tests/mailbox/test_mailbox_ring_restricted.c
tests/mailbox/test_mailbox_supervisor_endpoint_denied.c
tests/mailbox/test_mailbox_quarantine_isolation.c
```

### test_mailbox_ring_restricted

Proves mailbox delivery cannot bypass ring policy.

Valid mailbox structure is not enough.

### test_mailbox_supervisor_endpoint_denied

Proves lower rings cannot send arbitrary commands to Supervisor-owned mailbox endpoints.

Supervisor requests must go through explicit service policy.

### test_mailbox_quarantine_isolation

Proves quarantined nodes cannot use otherwise valid mailbox traffic to reach trusted nodes.

## Fabric/Ring Interaction Tests

Planned test files:

```text
tests/sim/test_fabric_ring_routing.c
tests/sim/test_fabric_quarantine_node.c
tests/sim/test_fabric_supervisor_visibility.c
```

### test_fabric_ring_routing

Proves fabric routing consults ring policy before delivery.

A valid route is not sufficient if the ring boundary denies communication.

### test_fabric_quarantine_node

Proves the Supervisor can quarantine a node and the fabric prevents normal traffic to or from that node.

### test_fabric_supervisor_visibility

Proves Ring -2 can observe fabric topology while less privileged rings see only what service policy exposes.

## Suggested Test Count Milestones

Current baseline:

```text
29 tests passing
```

Ring Manager milestone:

```text
+5 ring tests
34 tests expected
```

Capability/ring interaction milestone:

```text
+3 capability/ring tests
37 tests expected
```

Mailbox/ring interaction milestone:

```text
+3 mailbox/ring tests
40 tests expected
```

Fabric/ring interaction milestone:

```text
+3 fabric/ring tests
43 tests expected
```

## Implementation Ordering

Recommended order:

```text
1. Ring Manager public interface
2. Ring Manager core tests
3. Capability/ring integration
4. Mailbox/ring integration
5. Fabric/ring integration
6. Supervisor service registry
```

Do not build Supervisor service registration before ring policy has executable tests.

## Result

This test plan makes the ring-security doctrine actionable.

Ring security becomes a required, testable boundary rather than an implied design intention.
