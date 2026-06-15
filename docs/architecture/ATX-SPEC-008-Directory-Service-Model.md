# ATX-SPEC-008 Directory Service Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Directory Service Model.

The Directory Service maps human-readable names to object identities. It provides discovery, binding, aliasing, enumeration, lookup, and stale-binding handling for Atarix objects.

The Directory Service does not grant authority.

Lookup is not access.

Knowing a name is not authority.

Resolving an object identity is not authority.

All operations on resolved objects remain subject to ring policy, ownership rules, capability validation, lifecycle state, resource policy, and system policy.

## Design Philosophy

Atarix separates:

```text
Name
Binding
Object identity
Object location
Authority
Capability
Lifecycle state
Resource state
```

The Directory Service answers:

```text
Given this name, what object identity does it currently bind to?
```

It does not answer:

```text
May this caller use the object?
```

That question belongs to the Security, Authority, Capability, Policy, Lifecycle, and Resource models.

## Core Invariants

1. Names are not objects.
2. Names are not capabilities.
3. Object identities are not names.
4. Object identities are not authority.
5. Lookup success does not imply access.
6. Enumeration success does not imply access.
7. Directory visibility does not imply control.
8. Bindings have owners.
9. Bindings have lifecycle state.
10. Bindings have generation metadata.
11. Bindings can become stale.
12. Stale bindings must be observable.
13. Stale bindings must be removable or tombstoned.
14. Destroyed object identities must never be reused to satisfy stale bindings.
15. Directory operations must be auditable.
16. Directory failure must fail closed for authority-bearing operations.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## Terminology

### Name

A human-readable identifier within a namespace.

Example:

```text
/system/directory
/service/audit
/fabric/node/0
/device/network/eth0
/user/donovan/session/0
```

### Binding

A directory record that maps a name or path to an object identity.

### Object Identity

An opaque immutable identity defined by the Object Model.

### Alias

A binding that resolves to another binding or canonical object path.

### Directory Entry

A record containing binding metadata.

### Directory Object

An object that stores and manages directory entries for a namespace subtree.

### Stale Binding

A binding whose target object no longer exists, cannot be verified, has moved without update, or is otherwise invalid.

### Canonical Path

The preferred human-readable path for an object.

An object may have multiple names, but at most one canonical path within a given namespace authority.

## Operational Fabric Plane

The Directory Service operates on the Operational Fabric discovery and service planes.

It is not part of the bulk data plane.

It is not supervisor control authority.

The Operational Fabric may request information from directory services, but directory lookup remains separate from control authority and resource authority.

## Top-Level Namespace Layout

The recommended top-level namespace layout is:

```text
/
  system/
  service/
  device/
  fabric/
  user/
  session/
  temporary/
  audit/
```

## Binding Model

The core model is:

```text
Name
  -> Binding
  -> Object Identity
  -> Object
```

A binding must contain at least:

```text
Binding ID
Name / path component
Target object identity
Binding type
Owner
Creator
Lifecycle state
Generation
Creation time
Last update time
Persistence policy
Lease / expiration policy
Audit policy
Version / schema information
```

## Binding Types

Atarix recognizes the following conceptual binding types:

```text
DIRECT
ALIAS
MOUNT
REDIRECT
TOMBSTONE
STALE
```

Aliases, mounts, and redirects must not create authority or bypass capability checks.

## Lookup Semantics

A lookup operation resolves a name into metadata.

Conceptually:

```text
lookup("/system/directory")
```

returns:

```text
Object identity
Object type hint
Binding ID
Binding generation
Owner
Lifecycle state
Policy reference
Directory authority
Version information
```

Lookup does not return a capability.

Lookup does not grant object access.

Lookup does not imply that future lookup of the same name resolves to the same object unless a binding generation or object identity is pinned.

## Lookup Failure

Lookup may fail because:

```text
Name not found
Malformed path
Permission denied to enumerate or resolve
Directory unavailable
Binding stale
Binding expired
Alias loop detected
Redirect denied
Target object destroyed
Policy unavailable
Unsupported version
Unknown state
```

Authority-bearing operations must fail closed when lookup state cannot be verified.

## Lookup And Authority

The following are explicitly false:

```text
Knowing a path grants authority.
Resolving a path grants authority.
Enumerating a directory grants authority.
Owning a name grants authority over the target object.
Owning an object automatically grants authority over all names bound to it.
```

Authority must be checked separately.

The access path is:

```text
Lookup
  -> Object identity
  -> Ring policy
  -> Ownership / authority policy
  -> Capability validation
  -> Lifecycle validation
  -> Resource policy
  -> Operation
  -> ALLOW / DENY
```

## Directory Visibility

Atarix may allow directory entries to be visible without granting object access.

Visibility itself may still be policy-controlled.

Observation is not control.

## Generation And Version

Binding generation identifies binding evolution.

Version identifies schema or protocol compatibility.

These must not be conflated.

Rebinding must increment generation metadata and create an audit event.

Unsupported binding or reply versions must fail closed for authority-bearing operations unless explicit compatibility rules allow degraded interpretation.

## Lifecycle Integration

Directory entries participate in the Lifecycle Model.

Bindings may be:

```text
CREATED
ACTIVE
SUSPENDED
STALE
EXPIRED
REMOVED
TOMBSTONED
QUARANTINED
UNKNOWN
```

When an object is destroyed:

```text
Object destroyed
  -> capabilities revoked or invalidated
  -> mailboxes closed
  -> directory bindings marked stale, removed, or tombstoned
  -> resources reclaimed
  -> audit record generated
```

No binding may silently resolve to a replacement object after target destruction.

## Temporary And Persistent Bindings

Temporary bindings are lease-oriented by default.

Persistent bindings require explicit policy, recovery behavior, cleanup authority, version handling, and audit visibility.

Persistence must not be accidental.

## Distributed Directory Behavior

The Directory Service must support future distributed operation.

A lookup may eventually resolve to:

```text
Local object
Object on another node in the same fabric
Migrated object
Replicated service
Remote namespace authority
Federated fabric gateway
```

Rev A requires only one operational fabric.

Remote resolution does not imply remote access.

## Service Location Independence

Services are discovered by name, interface, version, and capability requirements, not by physical implementation.

A service may be provided by a CPU card, auxiliary compute card, storage processor, network card, or future fabric node.

Service location is not service identity.

## Bootstrap Considerations

Some early boot services may need name-like resolution before the full Directory Service exists.

That behavior belongs to the Bootstrap Security Model.

Bootstrap lookup must not be treated as normal runtime directory authority.

Bootstrap authority is not runtime authority.

## POSIX Compatibility Considerations

Future POSIX compatibility will map POSIX paths onto Atarix namespace and directory operations.

POSIX path lookup must not import POSIX ambient-authority assumptions into Atarix.

## Audit Requirements

The following operations must be auditable:

```text
Create binding
Remove binding
Rename binding
Rebind name
Create alias
Remove alias
Create mount
Remove mount
Directory lookup denied
Directory lookup failed due to stale target
Directory lookup redirected
Directory cache invalidated
Binding expired
Binding tombstoned
Binding quarantined
Unsupported version rejected
```

## Initial Directory Sprint Scope

Directory Sprint 1 should implement only:

```text
Direct bindings
Human-readable paths
Object identity targets
Lookup
Create binding
Remove binding
Generation counters
Stale binding behavior
Basic tests
```

Directory Sprint 1 should not implement distributed lookup, replication, caching, POSIX path emulation, complex mounts, or policy language.

## Required Tests

Initial tests should verify:

```text
Create binding
Lookup existing binding
Lookup missing binding
Remove binding
Removed binding does not resolve
Rebinding increments generation
Alias loop rejected
Lookup does not grant authority
Stale binding rejected
Destroyed object identity not reused
Malformed path rejected
Temporary binding expiration
Directory mutation requires authority
Unsupported version fails closed
```

## Open Questions

Q-001: What exact path syntax is allowed?

Q-002: What maximum path length should be supported?

Q-003: What maximum component length should be supported?

Q-004: Are paths case-sensitive?

Q-005: Are Unicode names allowed in kernel-visible paths, or only higher-level display names?

Q-006: How are binding IDs represented?

Q-007: What minimum metadata must fit in a wire-format directory reply?

Q-008: Which namespace roots are mandatory at boot?

Q-009: How does the Directory Service recover persistent bindings after crash?

Q-010: What exact audit event format is required?

## Summary

The Directory Service provides discoverability, not authority.

Its central rule is:

```text
Lookup is not access.
```
