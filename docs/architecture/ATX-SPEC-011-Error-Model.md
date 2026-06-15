# ATX-SPEC-011 Error Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Error Model.

The Error Model provides a consistent architectural framework for handling failures, uncertainty, degradation, recovery, quarantine, reconciliation, and fault reporting.

Errors are architectural events.

Errors are not merely return codes.

Failures are expected.

Unknown state is a first-class condition.

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
16. Compatibility failures must not weaken native semantics.

## Error Categories

Conceptual categories include:

```text
Validation errors
Authority errors
Resource errors
Lifecycle errors
Directory errors
Communication errors
Storage errors
Audit errors
Supervisor errors
Bootstrap errors
Compatibility errors
Version errors
Policy errors
Recovery errors
```

## Severity

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

Unknown state is distinct from failure and success.

Examples:

```text
Storage partially recovered
Node restarted unexpectedly
Resource ownership uncertain
Audit continuity uncertain
Directory reconciliation incomplete
Capability state uncertain
Version compatibility unknown
Policy unavailable
```

Unknown state must not be silently treated as valid.

Unknown state may require quarantine, restricted operation, manual review, recovery workflow, or denial.

## Quarantine

Quarantine isolates potentially unsafe state while preserving evidence.

Quarantine may apply to:

```text
Objects
Resources
Directory bindings
Capabilities
Mailbox endpoints
Services
Persistent storage
Audit streams
Compatibility environments
```

Quarantine must be auditable.

## Fail-Closed Rules

Authority-bearing operations must fail closed when:

```text
Authority cannot be verified
Capability cannot be validated
Policy cannot be evaluated
Directory target cannot be verified
Resource ownership is uncertain
Lifecycle state is unknown
Version semantics are unknown
Audit-critical provenance is invalid
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
Recovery-only mode
Compatibility fallback mode
```

Degraded operation must be explicit, auditable, observable, and policy controlled.

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

After crash, partition, storage rollback, audit discontinuity, or version mismatch, Atarix may require reconciliation.

Reconciliation may apply to:

```text
Resource ownership
Directory bindings
Audit continuity
Lease state
Mailbox state
Capability state
Persistent storage state
Service provider state
```

Reconciliation must not assume correctness.

## Error Reporting Requirements

Every error should include:

```text
Error category
Error code
Subsystem
Object identity if applicable
Authority context
Timestamp
Severity
Recovery recommendation
Version or schema identifier
Audit correlation ID
```

## Audit Integration

Significant errors must generate audit events.

Particularly:

```text
Authority failures
Recovery actions
Quarantine actions
Resource exhaustion
Supervisor failures
Audit failures
Bootstrap failures
Version failures
Policy failures
Compatibility failures
Cleanup failures
```

## Supervisor Integration

Supervisor failures should be exported through the Supervisor Audit Bridge.

Supervisor control isolation remains in effect.

Observation is not control.

## Resource Exhaustion

Resource exhaustion is expected.

The system should preserve recovery paths, supervisor functions, and audit visibility where practical.

Resource exhaustion must not bypass authority checks.

## Bootstrap Errors

Bootstrap failures belong to the Bootstrap Security Model.

Examples:

```text
Firmware validation failure
Boot image failure
Netboot failure
Time source unavailable
Supervisor startup failure
```

Bootstrap error handling must not grant runtime authority.

## Storage And Persistence Errors

Storage uncertainty must be explicit.

Examples:

```text
Checksum mismatch
Journal replay required
Snapshot rollback
Partial write
Pool unavailable
Unknown dataset generation
```

Unknown persistent state should be quarantined or mounted read-only until reconciled.

## Compatibility Errors

Compatibility layers must fail explicitly when legacy assumptions conflict with native Atarix security semantics.

Compatibility failure must not weaken authority, lifecycle, resource, audit, or cleanup rules.

## Crash Recovery

After a crash:

```text
Detect
Audit
Quarantine uncertainty
Reconcile state
Restore services
```

Unknown state remains visible until resolved.

## Initial Error Sprint Scope

Error Sprint 1 should define:

```text
Error categories
Error codes
Severity model
Unknown state support
Quarantine support
Basic tests
```

## Required Future Work

- Define concrete error code namespace.
- Define structured error object format.
- Define quarantine transition table.
- Define recovery workflow in ATX-SPEC-018.
- Define version error handling in ATX-SPEC-012.
- Define policy error handling in ATX-SPEC-013.
- Define storage error handling in ATX-SPEC-017.

## Summary

The Error Model treats failure, uncertainty, recovery, and quarantine as first-class architecture.

The central rules are:

```text
Unknown state must be explicit.
When authority cannot be verified, fail closed.
```
