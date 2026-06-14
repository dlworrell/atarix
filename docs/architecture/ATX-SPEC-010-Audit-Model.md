# ATX-SPEC-010 Audit Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Audit Model.

The Audit Model provides accountability, observability, provenance, incident reconstruction, lifecycle visibility, resource accountability, and security-event recording across the entire Atarix architecture.

Audit is a first-class architectural subsystem.

Audit is not debugging.

Audit is not logging.

Audit is evidence.

## Design Philosophy

Atarix assumes:

- Compromise may occur.
- Software may fail.
- Authority may be abused.
- Resources may leak.
- Cleanup may fail.
- Systems may crash.
- Administrators may need to reconstruct events long after they occurred.

The Audit Model exists to answer:

```text
What happened?
Who did it?
What authority was used?
What object was affected?
When did it occur?
Why was it allowed?
What changed?
Can we prove it?
```

## Core Principles

1. Audit is architectural.
2. Audit is append-oriented.
3. Audit records are evidence.
4. Observation is not control.
5. Audit visibility does not imply authority.
6. Audit records must have provenance.
7. Audit records must identify authority used.
8. Audit records must survive routine failures where practical.
9. Audit suppression must be observable.
10. Audit exhaustion must not silently disable auditing.
11. System objects are not exempt.
12. Supervisor events are auditable.
13. Cleanup failures are auditable.
14. Resource exhaustion is auditable.
15. Security failures are auditable.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model

This specification informs:

- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- Future Supervisor Management Fabric specification

## Core Rule

Audit authority and operational authority are separate.

The ability to read audit data does not imply the ability to modify system state.

The ability to modify system state does not imply the ability to modify audit history.

Observation is not control.

## Audit Architecture

Conceptually:

```text
Objects
Services
Resources
Directory
Lifecycle
Capabilities
Supervisor Fabric
        ↓
Audit Events
        ↓
Audit Service
        ↓
Audit Storage
        ↓
Audit Queries
```

Audit should be treated as a service architecture rather than a collection of log files.

## Audit Domains

Atarix recognizes multiple audit domains.

### Security Domain

Examples:

```text
Capability granted
Capability revoked
Delegation
Authorization denied
Authorization allowed
Policy violation
Ring boundary violation
```

### Lifecycle Domain

Examples:

```text
Object created
Object destroyed
Lease expired
Cleanup started
Cleanup failed
Ownership changed
```

### Resource Domain

Examples:

```text
Allocation
Reservation
Quota denial
Resource exhaustion
Resource reclamation
```

### Directory Domain

Examples:

```text
Binding created
Binding removed
Alias created
Rebinding
Lookup denied
Lookup redirected
```

### Supervisor Domain

Examples:

```text
Reset
Watchdog event
Firmware validation
Recovery mode
Power event
RTC adjustment
```

### Fabric Domain

Examples:

```text
Node joined
Node removed
Fabric partition
Route failure
Transport fault
```

### Administrative Domain

Examples:

```text
Policy change
Configuration change
Software install
Software removal
Upgrade
Rollback
```

## Audit Record Requirements

Every audit record should contain:

```text
Audit record ID
Timestamp
Timestamp source
Event type
Actor identity label
Authority used
Object identity
Operation
Result
Reason
```

Additional metadata may include:

```text
Resource ID
Binding ID
Capability ID
Policy ID
Ring
Node
Fabric
Session
Version
Correlation ID
```

## Provenance

Every audit record must identify:

```text
Who generated the record
What authority generated it
What subsystem generated it
```

Audit records without provenance are lower trust.

## Audit Integrity

Audit records should be:

```text
Append-oriented
Sequence numbered
Tamper evident
Ordered where practical
Correlatable
```

A compromised application should not be able to rewrite historical audit records.

## Audit Visibility

Audit data is observable.

Audit visibility may be policy controlled.

Visibility does not imply control.

Audit consumers may include:

```text
Administrators
Security services
Supervisor services
Monitoring systems
Diagnostic tools
Applications (restricted)
```

## Audit Queries

Audit queries may support:

```text
By object
By owner
By resource
By capability
By authority
By time
By session
By node
By event type
```

Audit query access is policy controlled.

## Supervisor Audit Bridge

The Supervisor Management Fabric is control-isolated.

Supervisor events may be exported through a Supervisor Audit Bridge.

Conceptually:

```text
Supervisor Fabric
    ↓
Audit Bridge
    ↓
Operational Audit Service
```

The bridge is observational.

The bridge does not grant operational control over supervisor resources.

Examples of exported events:

```text
Watchdog timeout
Boot validation failure
Recovery entry
Power fault
Firmware update
RTC synchronization
```

## Audit And Resource Exhaustion

Audit must remain observable during resource pressure.

If audit buffers are exhausted:

```text
Audit loss must be recorded.
Audit suppression must be visible.
Audit degradation must be visible.
```

The system must not silently stop auditing.

## Audit And Cleanup

Cleanup operations are auditable.

Examples:

```text
Lease expiration
Object destruction
Mailbox cleanup
Resource reclamation
Persistent state removal
```

Cleanup failures are security-relevant events.

## Audit And Security Events

The following should always generate audit records:

```text
Capability grant
Capability revoke
Delegation
Authorization denial
Policy override
Ring violation
Supervisor action
Recovery mode entry
```

## Audit And Crashes

After crash recovery, audit systems should record:

```text
Crash detected
Recovery started
Recovery completed
Unreconciled state
Quarantined resources
```

Unknown state should be visible.

## Audit And Bootstrap

Some audit events occur before the runtime Audit Service exists.

Examples:

```text
Boot ROM validation
Supervisor startup
Fabric initialization
Netboot fetch
Firmware validation
```

These belong to the Bootstrap Security Model.

Bootstrap audit records should be imported into the runtime audit stream where practical.

## Audit And Distributed Systems

Future fabrics may contain multiple audit producers.

Audit records should support:

```text
Node identification
Fabric identification
Correlation IDs
Independent provenance
```

Rev A only requires one operational fabric.

## Audit Storage

Audit storage may be:

```text
Memory-backed
Persistent
Remote
Supervisor-protected
Replicated
```

Storage implementation is separate from audit semantics.

Audit storage must preserve provenance and integrity information.

## Audit Retention

Retention is policy controlled.

Retention policies may include:

```text
Time-based
Size-based
Event-class-based
Compliance-based
Administrative
```

Deletion of audit data should itself be auditable.

## Audit Authority

Audit authority should distinguish:

```text
Observe audit
Query audit
Export audit
Manage retention
Purge audit
```

These are separate authorities.

## Failure Rules

When audit certainty decreases, uncertainty must be visible.

Examples:

```text
Timestamp unavailable
Storage unavailable
Buffer exhausted
Producer unavailable
Supervisor disconnected
```

The system must not fabricate certainty.

## Initial Audit Sprint Scope

Audit Sprint 1 should implement:

```text
Audit record structure
Event categories
Basic audit service
Append-only event recording
Query by event type
Query by object
Basic tests
```

Audit Sprint 1 should not implement:

```text
Distributed correlation
Remote audit federation
Cryptographic signing
Replication
Advanced retention
Cross-fabric audit
```

## Required Tests

Initial tests should verify:

```text
Record audit event
Query audit event
Append ordering
Record capability grant
Record capability revoke
Record resource allocation
Record object creation
Record object destruction
Record directory mutation
Record authorization denial
Record cleanup event
Record audit exhaustion event
Supervisor audit import path
```

## Open Questions

Q-001: What exact audit record wire format should be used?

Q-002: Are audit records objects?

Q-003: Should audit records have immutable object identities?

Q-004: What minimum timestamp precision is required?

Q-005: How should supervisor audit records be imported?

Q-006: What integrity guarantees are required in Rev A?

Q-007: What audit metadata is mandatory?

Q-008: What events are required during bootstrap?

Q-009: How should audit survive storage exhaustion?

Q-010: What audit query language is required?

## Summary

The Audit Model provides evidence, provenance, accountability, and observability across Atarix.

Its central rule is:

```text
Observation is not control.
```

Its companion rule is:

```text
Audit is evidence.
```

Audit exists so that administrators, services, and future recovery systems can answer:

```text
What happened?
```

with confidence.