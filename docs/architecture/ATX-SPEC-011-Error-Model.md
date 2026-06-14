# ATX-SPEC-011 Error Model

## Status

Draft v0.1

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

## Error Categories

### Validation Errors

```text
Malformed record
Invalid capability
Invalid path
Version mismatch
Schema violation
```

### Authority Errors

```text
Access denied
Capability missing
Capability expired
Delegation denied
Ring violation
Policy violation
```

### Resource Errors

```text
Out of memory
Quota exceeded
Mailbox exhausted
Storage exhausted
DMA unavailable
Allocation denied
```

### Lifecycle Errors

```text
Object destroyed
Object expired
Lease expired
Cleanup failure
Invalid state transition
```

### Directory Errors

```text
Name not found
Binding stale
Alias loop
Redirect denied
Namespace unavailable
```

### Communication Errors

```text
Mailbox timeout
Transport failure
Message corruption
Node unavailable
Partition detected
```

### Storage Errors

```text
Read failure
Write failure
Checksum mismatch
Journal replay required
Pool unavailable
```

### Audit Errors

```text
Audit storage unavailable
Audit buffer exhausted
Audit import failure
Audit integrity failure
```

### Supervisor Errors

```text
Watchdog event
Firmware validation failure
Recovery entry
Power fault
RTC fault
Supervisor unavailable
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
```

Unknown state must not be silently treated as valid.

Unknown state may require:

```text
Quarantine
Restricted operation
Manual review
Recovery workflow
```

## Quarantine

Quarantine isolates potentially unsafe state.

Examples:

```text
Unverified storage
Unreconciled resource
Compromised service
Unknown capability state
Corrupted audit stream
```

Quarantine preserves evidence while preventing unsafe operation.

## Fail-Closed Rules

Authority-bearing operations must fail closed when:

```text
Authority cannot be verified
Capability cannot be validated
Policy cannot be evaluated
Directory target cannot be verified
Resource ownership is uncertain
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
```

Degraded operation must be:

```text
Explicit
Auditable
Observable
Policy controlled
```

## Recovery States

Suggested recovery states:

```text
NORMAL
DEGRADED
RECOVERING
QUARANTINED
EMERGENCY
FAILED
```

State transitions must be auditable.

## Reconciliation

After crashes or partitions, the system may require reconciliation.

Examples:

```text
Resource ownership
Directory bindings
Audit continuity
Lease state
Mailbox state
```

Reconciliation must not assume correctness.

## Error Reporting Requirements

Every error should include:

```text
Error category
Error code
Subsystem
Object identity (if applicable)
Authority context
Timestamp
Severity
Recovery recommendation
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
```

## Supervisor Integration

Supervisor failures should be exported through the Supervisor Audit Bridge.

Supervisor control isolation remains in effect.

Observation is not control.

## Resource Exhaustion

Resource exhaustion is expected.

The system should:

```text
Preserve recovery paths
Preserve supervisor functions
Preserve audit visibility where possible
Avoid authority bypasses
```

## Bootstrap Errors

Bootstrap failures belong to the Bootstrap Security Model.

Examples:

```text
Firmware validation failure
Boot image failure
Netboot failure
Time source unavailable
```

## Crash Recovery

After a crash:

```text
Detect
Audit
Quarantine uncertainty
Reconcile state
Restore services
```

Unknown state must remain visible until resolved.

## Initial Error Sprint Scope

```text
Error categories
Error codes
Severity model
Unknown state support
Quarantine support
Basic tests
```

## Required Tests

```text
Validation error
Authority error
Resource exhaustion
Directory failure
Quarantine transition
Recovery transition
Unknown state handling
Audit generation for errors
```

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
