# ATX-SPEC-010 Audit Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Audit Model.

Audit provides accountability, observability, provenance, incident reconstruction, lifecycle visibility, resource accountability, and security-event recording.

Audit is a first-class architectural subsystem.

Audit is not debugging.

Audit is not ordinary logging.

Audit is evidence.

## Core Principles

1. Audit is architectural.
2. Audit records are evidence.
3. Audit authority and operational authority are separate.
4. Observation is not control.
5. Audit visibility does not imply authority.
6. Audit records require provenance.
7. Audit suppression must be observable.
8. Audit exhaustion must be observable.
9. Cleanup failures are auditable.
10. Supervisor events are auditable.
11. Bootstrap events are auditable where practical.
12. Unknown audit continuity must be explicit.

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
Bootstrap Services
        ↓
Audit Events
        ↓
Audit Ingress
        ↓
Audit Service
        ↓
Audit Storage
        ↓
Audit Queries
```

## Audit Domains

Audit domains include:

```text
Security
Lifecycle
Resource
Directory
Supervisor
Fabric
Bootstrap
Administrative
Compatibility
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
Producer identity
Version or schema identifier
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
Correlation ID
Generation
Sequence number
```

## Provenance And Integrity

Every audit record must identify who generated the record, what authority generated it, and what subsystem generated it.

Audit records should be append-oriented, sequence-numbered, tamper-evident, ordered where practical, and correlatable.

The system must not fabricate certainty when provenance is unknown.

## Audit Visibility

Audit visibility is policy-controlled.

The ability to observe audit data does not imply the ability to control the audited component.

The ability to control a component does not imply the ability to alter audit history.

## Supervisor Audit Bridge

The Supervisor Management Fabric is control-isolated.

Supervisor events may be exported through a Supervisor Audit Bridge:

```text
Supervisor Fabric
    ↓
Audit Bridge
    ↓
Operational Audit Service
```

The bridge is observational.

The bridge does not grant operational control over supervisor resources.

## Audit Buffering And Persistence

Audit generation should not depend on synchronous physical disk writes.

A staged audit path is preferred:

```text
Audit producer
  -> Reserved high-speed audit buffer
  -> Audit persistence queue
  -> Persistent journal
  -> Checksummed storage
  -> Archive or replication target
```

Rev A may begin with small supervisor and CPU-local audit buffers.

Fabric RAM cards enable larger RAM-backed audit buffers and replayable audit journals.

ZFS-style checksummed copy-on-write audit persistence is a long-term target, not a Rev A bootstrap dependency.

## Reserved Audit Resources

Audit should have reserved resources where practical:

```text
Reserved RAM
Reserved persistent storage
Reserved mailbox capacity
Reserved CPU budget
Reserved fabric bandwidth
Supervisor-local event buffer
```

Normal workload exhaustion must not silently consume all audit capacity.

## Audit Degradation

If audit storage, buffers, or producers fail:

```text
Audit loss must be recorded.
Audit suppression must be visible.
Audit degradation must be visible.
Unknown audit continuity must be explicit.
```

The system must not silently stop auditing.

## Audit And Cleanup

Cleanup operations are auditable.

Cleanup failures are security-relevant events.

## Audit And Errors

Significant errors must generate audit records, including authority failures, recovery actions, quarantine actions, resource exhaustion, supervisor failures, audit failures, and bootstrap failures.

## Audit And Bootstrap

Some audit events occur before the runtime Audit Service exists.

Bootstrap audit records should be imported into the runtime audit stream where practical.

Bootstrap audit import must preserve provenance and must not become runtime authority.

## Audit And Compatibility

Compatibility VMs and POSIX personalities must emit audit records for authority-bearing operations, resource allocation, persistent state creation, and cleanup failure.

Compatibility must not become an audit blind spot.

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

## Required Future Work

- Define audit record wire format.
- Define audit sequence-number semantics.
- Define supervisor audit import format.
- Define RAM-backed audit buffer hierarchy.
- Define persistent audit storage in ATX-SPEC-017.
- Define recovery reconciliation in ATX-SPEC-018.
- Define audit policy in ATX-SPEC-013.

## Summary

Audit exists so administrators, services, and recovery systems can answer:

```text
What happened?
```

with confidence.

The central rules are:

```text
Audit is evidence.
Observation is not control.
```
