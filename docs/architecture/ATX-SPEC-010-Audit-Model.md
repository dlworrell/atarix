# ATX-SPEC-010 Audit Model

## Status

Draft v0.2, reconciled after Architecture Review 001

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
What resource was consumed?
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
16. Audit generation should not depend on synchronous physical storage writes.
17. Audit storage state uncertainty must be explicit.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-011 Error Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model

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
Audit Ingress
        ↓
High-Speed Audit Buffer
        ↓
Audit Persistence Queue
        ↓
Audit Storage
        ↓
Audit Queries / Export
```

Audit should be treated as a service architecture rather than a collection of log files.

## Audit Domains

Atarix recognizes multiple audit domains:

```text
Security
Lifecycle
Resource
Directory
Supervisor
Fabric
Administrative
Bootstrap
Storage
Recovery
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
Record version
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
Generation
Correlation ID
Supervisor provenance
Storage state
```

## Provenance

Every audit record must identify:

```text
Who generated the record
What authority generated it
What subsystem generated it
Where it was generated
What version generated it
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
Versioned
Recoverable where practical
```

A compromised application should not be able to rewrite historical audit records.

## Audit Visibility

Audit data is observable according to policy.

Audit visibility does not imply control.

Audit consumers may include administrators, security services, supervisor services, monitoring systems, diagnostic tools, and restricted applications.

## Audit Buffering And Persistence

Audit generation must not require synchronous physical disk writes.

Audit events should first enter a high-speed append buffer.

Audit persistence should be asynchronous and journaled where practical.

Audit resources shall be reserved and isolated from normal workloads where practical.

Audit exhaustion shall be observable.

Supervisor audit buffering shall be independent of operational audit buffering.

Loss of persistent storage shall degrade audit persistence but must not silently disable audit generation.

## Audit Storage Hierarchy

Target architecture:

```text
L0  Supervisor audit buffer
L1  Operational audit ingress buffer
L2  RAM-backed audit journal
L3  Persistent audit journal
L4  Checksummed copy-on-write audit dataset
L5  Archive / replication target
```

Early Rev A implementations may support only supervisor-local fault logging and small operational ring buffers.

Fabric RAM cards or compatible memory services are expected before serious RAM-backed audit journals become practical.

## ZFS-Inspired Recovery Target

The long-term storage target should support ZFS-like properties:

```text
Checksums
Copy-on-write updates
Snapshots or immutable checkpoints
Replayable journal state
Scrub or verification
Send/receive or export equivalent
```

Direct ZFS use is not required for early Atarix, but the audit persistence model should not preclude ZFS or ZFS-like storage.

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

Cleanup failures are security-relevant events.

Examples:

```text
Lease expiration
Object destruction
Mailbox cleanup
Resource reclamation
Persistent state removal
Directory binding removal
```

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
Quarantine transition
Bootstrap security failure
Unsupported version rejection
```

## Audit And Crashes

After crash recovery, audit systems should record:

```text
Crash detected
Recovery started
Recovery completed
Unreconciled state
Quarantined resources
Audit continuity state
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

Audit records should support node identification, fabric identification, correlation IDs, independent provenance, and version information.

Rev A only requires one operational fabric.

## Audit Retention

Retention is policy controlled.

Deletion, truncation, export, or rollover of audit data should itself be auditable.

## Audit Authority

Audit authority should distinguish:

```text
Observe audit
Query audit
Export audit
Manage retention
Purge audit
Import supervisor audit
Verify audit integrity
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
Unsupported record version
Integrity check failed
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

Audit Sprint 1 should not implement distributed correlation, remote audit federation, cryptographic signing, replication, advanced retention, RAM-drive audit persistence, or cross-fabric audit.

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
Record unsupported audit version
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

Q-011: What minimum RAM-backed buffering level requires RAM cards?

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
