# ATX-SPEC-011 Error Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Error Model.

The Error Model provides a consistent architectural framework for handling failures, uncertainty, degradation, recovery, quarantine, reconciliation, and fault reporting throughout the system.

Errors are architectural events.

Errors are not merely return codes.

Failures are expected.

Unknown state is a first-class condition.

## Design Philosophy

Atarix assumes:

- Hardware fails.
- Software contains defects.
- Resources are exhausted.
- Networks partition.
- Storage becomes inconsistent.
- Audit systems degrade.
- Compromise may occur.
- Recovery is required.

The Error Model exists to answer:

```text
What failed?
How certain are we?
What is the blast radius?
Can operation continue?
Can the state be trusted?
Must the resource be quarantined?
Can recovery occur automatically?
Must authority be revoked?
What evidence exists?
```

## Core Principles

1. Failure is normal.
2. Unknown state is explicit.
3. Fail closed for authority-bearing operations.
4. Preserve evidence.
5. Prefer quarantine over assumption.
6. Degradation must be observable.
7. Recovery must be auditable.
8. Uncertainty must never be hidden.
9. Error handling must not create authority.
10. Recovery must not silently regrant authority.
11. Resource exhaustion is an error condition.
12. Cleanup failure is an error condition.
13. Supervisor failures are auditable.
14. Bootstrap failures are auditable.
15. Observation is not control.
16. Version mismatch is explicit.
17. Storage uncertainty is explicit.
18. Audit uncertainty is explicit.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model

## Error Categories

### Validation Errors

```text
Malformed record
Invalid capability
Invalid path
Version mismatch
Schema violation
Unsupported feature
```

### Authority Errors

```text
Access denied
Capability missing
Capability expired
Delegation denied
Ring violation
Policy violation
Authority unknown
```

### Resource Errors

```text
Out of memory
Quota exceeded
Mailbox exhausted
Storage exhausted
DMA unavailable
Allocation denied
Reserved resource unavailable
```

### Lifecycle Errors

```text
Object destroyed
Object expired
Lease expired
Cleanup failure
Invalid state transition
Object quarantined
```

### Directory Errors

```text
Name not found
Binding stale
Alias loop
Redirect denied
Namespace unavailable
Unsupported binding version
```

### Communication Errors

```text
Mailbox timeout
Transport failure
Message corruption
Node unavailable
Partition detected
Unsupported message version
```

### Storage Errors

```text
Read failure
Write failure
Checksum mismatch
Journal replay required
Pool unavailable
Snapshot rollback required
Partial write suspected
```

### Audit Errors

```text
Audit storage unavailable
Audit buffer exhausted
Audit import failure
Audit integrity failure
Audit continuity unknown
Unsupported audit record version
```

### Supervisor Errors

```text
Watchdog event
Firmware validation failure
Recovery entry
Power fault
RTC fault
Supervisor unavailable
Supervisor audit bridge unavailable
```

### Bootstrap Errors

```text
Boot ROM validation failure
Supervisor startup failure
Fabric initialization failure
Netboot failure
Time source unavailable
Recovery image unavailable
```

## Error Severity

Suggested severity levels:

```text
INFO
NOTICE
WARNING
ERROR
CRITICAL
FATAL
UNKNOWN
```

Severity does not determine authority.

Severity helps determine response and audit handling.

## Unknown State

Unknown state is a distinct architectural condition.

Examples:

```text
Storage partially recovered
Node restarted unexpectedly
Resource ownership uncertain
Audit continuity uncertain
Directory reconciliation incomplete
Capability revocation state uncertain
Policy version unsupported
```

Unknown state must not be silently treated as valid.

Unknown state may require quarantine, restricted operation, manual review, or recovery workflow.

## Quarantine

Quarantine isolates potentially unsafe state.

Examples:

```text
Unverified storage
Unreconciled resource
Compromised service
Unknown capability state
Corrupted audit stream
Unsupported-version object
```

Quarantine preserves evidence while preventing unsafe operation.

Quarantine does not imply destruction.

## Fail-Closed Rules

Authority-bearing operations must fail closed when:

```text
Authority cannot be verified
Capability cannot be validated
Policy cannot be evaluated
Directory target cannot be verified
Resource ownership is uncertain
Object lifecycle is unknown
Version semantics are unsupported
Audit requirements cannot be satisfied where mandatory
```

Convenience must not override security.

## Degraded Operation

Some services may continue in degraded mode.

Examples:

```text
Read-only storage
Reduced networking
Limited directory service
Reduced audit retention
Supervisor-only recovery mode
```

Degraded operation must be explicit, auditable, observable, policy controlled, and reversible where practical.

## Recovery States

Suggested recovery states:

```text
NORMAL
DEGRADED
RECOVERING
RECONCILING
QUARANTINED
EMERGENCY
FAILED
UNKNOWN
```

State transitions must be auditable.

## Reconciliation

After crashes, partitions, unsupported versions, or partial recovery, the system may require reconciliation.

Examples:

```text
Resource ownership
Directory bindings
Audit continuity
Lease state
Mailbox state
Capability state
Persistent storage state
```

Reconciliation must not assume correctness.

Recovery must not silently regrant authority.

## Error Reporting Requirements

Every error should include:

```text
Error category
Error code
Subsystem
Object identity if applicable
Resource identity if applicable
Authority context
Timestamp
Severity
Recovery recommendation
Version context if applicable
Audit correlation ID if available
```

## Audit Integration

All significant errors must generate audit events.

Particularly:

```text
Authority failures
Recovery actions
Quarantine actions
Resource exhaustion
Supervisor failures
Audit failures
Bootstrap failures
Unsupported version rejections
Storage integrity failures
```

## Supervisor Integration

Supervisor failures should be exported through the Supervisor Audit Bridge.

Supervisor control isolation remains in effect.

Observation is not control.

## Resource Exhaustion

Resource exhaustion is expected.

The system should preserve recovery paths, supervisor functions, and audit visibility where possible.

Resource exhaustion must not create authority bypasses.

## Bootstrap Errors

Bootstrap failures belong to the Bootstrap Security Model.

Bootstrap errors should be imported into runtime audit where practical.

Bootstrap error handling must not grant runtime authority.

## Storage And Persistence Errors

Storage errors may require replay, rollback, quarantine, scrub, import refusal, or manual intervention.

Unknown storage state must remain visible.

Partially committed audit or object state must be replayed, discarded, or quarantined deterministically by future Storage and Recovery specifications.

## Crash Recovery

After a crash:

```text
Detect
Audit
Quarantine uncertainty
Reconcile state
Restore services
Record recovery result
```

Unknown state must remain visible until resolved.

## Initial Error Sprint Scope

Error Sprint 1 should implement:

```text
Error categories
Error codes
Severity model
Unknown state support
Quarantine support
Basic tests
```

Error Sprint 1 should not implement full recovery workflows, storage replay, distributed reconciliation, or policy language.

## Required Tests

Initial tests should verify:

```text
Validation error
Authority error
Resource exhaustion
Directory failure
Quarantine transition
Recovery transition
Unknown state handling
Audit generation for errors
Unsupported version failure
Fail-closed authority check
```

## Open Questions

Q-001: What exact error code namespace should be used?

Q-002: Are error records objects?

Q-003: How are errors correlated across services?

Q-004: What error severity is required for supervisor action?

Q-005: How does degraded operation interact with policy?

Q-006: What is the minimum quarantine API?

Q-007: How are bootstrap errors imported into runtime audit?

Q-008: What storage errors require immediate quarantine?

## Summary

The Error Model treats failure, uncertainty, recovery, and quarantine as first-class architectural concepts.

Its central rule is:

```text
Unknown state must be explicit.
```

Its companion rule is:

```text
When authority cannot be verified, fail closed.
```
