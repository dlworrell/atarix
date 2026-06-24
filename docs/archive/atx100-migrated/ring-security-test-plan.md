# ATARIX Ring Security Test Plan

Status: Planned

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

Initial rings:

```text
Ring 0: Supervisor / Root Authority
Ring 1: Fabric Management / Trusted System Services
Ring 2: Device Services / Card Firmware
Ring 3: Applications / User Workloads
Ring 4: Untrusted or Quarantined Nodes
```

The exact ring set may evolve by ADR. The existence of ring boundaries is mandatory.

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
Ring 3 -> Ring 0: denied
Ring 2 -> Ring 0: denied unless explicitly modeled as a supervised request path
Ring 4 -> any trusted ring: denied
```

### test_ring_downward_allowed

Proves higher-trust rings may supervise, constrain, or revoke authority from lower-trust rings.

Expected behavior:

```text
Ring 0 -> Ring 1: allowed for Supervisor operations
Ring 0 -> Ring 2: allowed for Supervisor operations
Ring 1 -> Ring 2: allowed for Fabric-management operations
```

### test_ring_quarantine_denied

Proves quarantined nodes cannot reach trusted services or other normal nodes.

Expected behavior:

```text
Ring 4 -> Ring 0: denied
Ring 4 -> Ring 1: denied
Ring 4 -> Ring 2: denied
Ring 4 -> Ring 3: denied unless explicit quarantine-service policy allows it
```

### test_ring_supervisor_override

Proves Ring 0 can perform recovery, revocation, and quarantine actions that lower rings cannot perform.

Expected behavior:

```text
Ring 0 may quarantine lower rings.
Ring 0 may revoke lower-ring authority.
Ring 0 may inspect Supervisor-owned tables.
Lower rings may not override Ring 0 policy.
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
Ring 3 application holds a valid-looking capability for a Ring 0 endpoint.
Ring policy denies before resource access.
```

### test_delegation_cannot_cross_ring_boundary

Proves delegation cannot increase authority across a prohibited ring boundary.

Example:

```text
Ring 2 device service delegates to Ring 3 application.
Delegated capability may only reduce authority.
Delegation cannot grant Ring 3 direct access to Ring 1 or Ring 0 internals.
```

### test_revoked_capability_still_denied_after_ring_check

Proves revocation remains effective even when ring policy would otherwise allow the request.

Example:

```text
Ring 3 -> Ring 2 permitted by ring policy.
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

Proves Ring 0 can observe fabric topology while lower rings see only what service policy exposes.

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
