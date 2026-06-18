# Capability Sprint 1 Review

Status: Complete

## Objective

Capability Sprint 1 established the first tested capability-policy foundation for ATARIX.

The sprint translated the accepted capability evaluation model into public headers, implementation code, and automated regression tests.

## Implemented Components

Public API:

```text
include/atarix/capability.h
include/atarix/capability_engine.h
include/atarix/capability_policy.h
```

Implementation:

```text
src/capability/capability_evaluator.c
src/capability/capability_policy.c
```

Tests:

```text
tests/capability/test_capability_validation.c
tests/capability/test_capability_ownership.c
tests/capability/test_capability_ring_boundary.c
tests/capability/test_capability_delegation.c
tests/capability/test_capability_revocation.c
```

## Security Invariants Verified

Capability Sprint 1 verifies the following security invariants:

```text
Validation
Ownership
Ring boundaries
Delegation
Revocation
Expiration
Deny-by-default behavior
```

## Validation

Capability records are rejected when structurally invalid.

The validation tests cover:

```text
NULL capability records
Invalid capability identifiers
Invalid operations
Invalid ring ranges
Missing target handles
Missing issuer handles
```

## Ownership

Capabilities are principal-bound.

The ownership tests cover:

```text
Caller-owner match allowed
Caller-owner mismatch denied
Revoked owner trust denied
Untrusted issuer denied
```

## Ring Boundaries

Ring boundaries are enforced as absolute authorization boundaries.

The ring tests cover:

```text
Application self-access
Service self-access
Driver self-access
Kernel self-access
Fabric self-access
Supervisor self-access
Application denied Kernel authority
Service denied Driver authority
Driver denied Fabric authority
Kernel denied Supervisor authority
```

## Delegation

Delegation may reduce authority only.

The delegation tests cover:

```text
Kernel to Driver allowed
Driver to Service allowed
Service to Application allowed
Application to Service denied
Service to Driver denied
Driver to Kernel denied
Delegation without delegation flag denied
Supervisor authority delegation denied
Delegation that adds flags denied
```

## Revocation and Expiration

Revocation and expiration are enforced before ordinary trust, ownership, ring, resource, and operation checks.

The revocation tests cover:

```text
Valid capability allowed
Revoked capability denied
Expired capability denied
Expiration boundary allowed
Zero lifetime denied
Revocation overrides ownership
Revocation overrides trust
```

## CI Result

Capability Sprint 1 completed with the following automated test result:

```text
TOTAL: 14
PASS:  14
SKIP:  0
XFAIL: 0
FAIL:  0
XPASS: 0
ERROR: 0
```

Test inventory:

```text
Discovery tests:   9
Capability tests:  5
Total tests:      14
```

## Architectural Decisions Implemented

The sprint implements the capability evaluation model established by:

```text
docs/adr/ADR-00X-capability-evaluation-model.md
```

Core decisions implemented:

```text
Supervisor is the root of trust.
Fabric is the enforcement authority.
Capabilities are principal-bound.
Capabilities are non-transferable.
Delegation may only reduce authority.
Revocation is authoritative.
Expiration is enforced.
Default decision is DENY.
```

## Deferred Work

The following work is intentionally deferred to later capability or directory sprints:

```text
Global atarix_status_t
Capability issuance workflow
Capability chain storage
Capability revocation registry
Supervisor-to-Fabric revocation propagation
Cryptographic capability integrity
Audit log integration
Capability serialization hardening
Cross-subsystem Directory integration
```

## Sprint 2 Recommendations

Recommended next capability work:

```text
Define capability issuance semantics.
Add capability attenuation helpers.
Define revocation registry representation.
Integrate capability policy with Directory Sprint 1.
Introduce global status handling when statuses cross subsystem boundaries.
Add audit-friendly reason codes.
```

## Result

Capability Sprint 1 is complete.

The ATARIX repository now has a tested capability-policy baseline with validation, ownership, ring, delegation, revocation, and expiration coverage.
