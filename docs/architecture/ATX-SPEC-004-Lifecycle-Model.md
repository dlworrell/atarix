# ATX-SPEC-004 Lifecycle Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines lifecycle behavior for Atarix objects, resources, bindings, capabilities, mailboxes, services, and persistent state.

Lifecycle is a security concern.

An object that cannot be cleaned up, revoked, audited, or reconciled becomes permanent attack surface.

## Core Rule

```text
No object is immortal by default.
```

Every object and resource must have an owner, scope, lifetime, cleanup path, persistence policy, audit visibility, and failure behavior.

## Lifecycle Axioms

```text
Persistence is not ownership.
Allocation is not ownership.
Ownership is not authority.
Unknown state must be explicit.
Recovery must not silently regrant authority.
Cleanup failure is a security event.
```

## Suggested Lifecycle States

General object lifecycle states:

```text
CREATED
INITIALIZING
ACTIVE
SUSPENDED
QUIESCING
REVOKING
REVOKED
EXPIRED
DESTROYING
DESTROYED
QUARANTINED
RECOVERING
FAILED
UNKNOWN
```

Not every object supports every state.

State transitions must be explicit and auditable.

## Ownership And Lifetime

Every object has an owner or responsible authority.

The owner is accountable for the object's existence and cleanup.

Ownership does not automatically grant every operation on the object.

Owner disappearance must be handled explicitly by policy.

## Leases

Temporary objects and resources should use leases by default.

A lease must define:

```text
Owner
Start time
Expiration time
Renewal authority
Expiration behavior
Cleanup authority
Audit behavior
```

Lease expiration must not silently become persistence.

## Persistence

Persistent state must be explicitly requested and policy-approved.

Persistent objects must define:

```text
Owner
Reason
Scope
Recovery behavior
Removal path
Audit visibility
Version or schema requirements
```

Persistence is not ownership and does not imply permanent authority.

## Cleanup

Cleanup may include:

```text
Capability revocation
Mailbox closure
Resource release
Directory binding removal
Temporary storage removal
DMA shutdown
Audit record generation
Cache invalidation
Persistent state removal or tombstoning
```

Cleanup failure must be visible to Audit and Error models.

## Quarantine

Quarantine isolates uncertain or unsafe state.

Quarantine may apply to:

```text
Objects
Resources
Directory bindings
Persistent storage
Capabilities
Services
Audit streams
```

Quarantine preserves evidence while preventing unsafe operation.

## Recovery

Recovery transitions an object or resource from failed, unknown, or quarantined state toward known safe state.

Recovery must not silently regrant authority.

Recovery must be auditable.

Recovered state must be distinguishable from uninterrupted normal state.

## Directory Integration

When an object is destroyed, directory bindings must be removed, marked stale, or tombstoned.

Destroyed object identities must not be reused to satisfy stale bindings.

## Resource Integration

When an object exits, expires, is destroyed, or loses authority, its resource allocations must be released, revoked, reconciled, or quarantined.

Resource leaks are architectural defects.

## Audit Integration

The following lifecycle events must be auditable:

```text
Object created
Object activated
Object suspended
Object expired
Object revoked
Object destroyed
Object quarantined
Object recovered
Cleanup started
Cleanup failed
Persistent state created
Persistent state removed
```

## Error Integration

Unknown lifecycle state must be explicit.

If lifecycle state cannot be verified, authority-bearing operations must fail closed.

## Supervisor And Bootstrap Integration

Supervisor objects and bootstrap objects are not exempt from lifecycle rules.

Bootstrap authority must not silently become runtime authority.

Supervisor control state must remain isolated while supervisor audit state remains observable where policy permits.

## Required Future Work

- Define concrete lifecycle state machine.
- Define required transitions for object, resource, capability, mailbox, and binding types.
- Define owner disappearance semantics.
- Define uninstall semantics.
- Define persistent-state cleanup semantics.
- Define recovery and reconciliation in ATX-SPEC-018.

## Summary

Lifecycle makes existence accountable.

If Atarix creates something, it must know who owns it, why it exists, how long it lives, how it is cleaned up, and what happens when cleanup fails.
