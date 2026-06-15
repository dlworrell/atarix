# ATX-SPEC-008 Directory Service Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Directory Service Model.

The Directory Service maps human-readable names to object identities. It provides discovery, binding, aliasing, enumeration, and lookup semantics for Atarix objects.

The Directory Service does not grant authority.

Lookup is not access.

Knowing a name is not authority.

Resolving an object identity is not authority.

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
Version state
```

The Directory Service answers:

```text
Given this name, what object identity does it currently bind to?
```

It does not answer:

```text
May this caller use the object?
```

That question belongs to the Security, Authority, Capability, Policy, Resource, and Lifecycle models.

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
10. Bindings have generation state.
11. Bindings can become stale.
12. Stale bindings must be observable.
13. Destroyed object identities must never be reused to satisfy stale bindings.
14. Directory operations must be auditable.
15. Directory failure must fail closed for authority-bearing operations.

## Relationship To Other Specifications

This specification depends on the Security, Authority, Lifecycle, Object, Namespace, Resource, Audit, and Error models.

This specification informs future Versioning, Policy, Bootstrap Security, Storage, Recovery, and Service specifications.

## Terminology

### Name

A human-readable identifier within a namespace.

### Binding

A directory record mapping a name or path to an object identity.

### Object Identity

An opaque immutable identity defined by the Object Model.

### Alias

A binding that resolves to another binding or canonical path.

### Stale Binding

A binding whose target object no longer exists, cannot be verified, has moved without update, or is otherwise invalid.

### Generation

A binding evolution counter.

Generation is not schema version.

Version describes compatibility. Generation describes instance history.

## Top-Level Namespace Layout

Recommended top-level namespace layout:

```text
/system
/service
/device
/fabric
/user
/session
/temporary
/audit
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
Name or path component
Target object identity
Binding type
Owner
Creator
Lifecycle state
Generation
Creation time
Last update time
Persistence policy
Lease or expiration policy
Audit policy
Version or schema identifier
```

## Binding Types

Conceptual binding types:

```text
DIRECT
ALIAS
MOUNT
REDIRECT
TOMBSTONE
STALE
```

Aliases, mounts, and redirects must not create authority or bypass policy.

## Lookup Semantics

Lookup returns metadata, not authority.

Conceptual result:

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
Unsupported version
Policy unavailable
```

Failure categories must map into the Error Model.

## Lookup And Authority

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

Directory entries may be visible without granting object access.

Visibility itself may still be policy-controlled.

Observation is not control.

## Aliases And Rebinding

Aliases must detect loops, preserve auditability, preserve policy boundaries, and never create authority.

Rebinding requires explicit authority, increments generation, and creates an audit event.

Clients requiring stability must pin object identity or binding generation.

## Lifecycle Integration

When an object is destroyed:

```text
Object destroyed
  -> capabilities revoked or invalidated
  -> mailboxes closed
  -> resources reclaimed or quarantined
  -> directory bindings marked stale, removed, or tombstoned
  -> audit record generated
```

No binding may silently resolve to a replacement object after target destruction.

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
```

## Distributed And Service Considerations

Rev A requires one operational fabric.

Future directory behavior may support local objects, remote objects within the same fabric, federated fabric references, migrated services, and replicated services.

Service location is not service identity.

## Bootstrap Considerations

Bootstrap lookup belongs to the Bootstrap Security Model.

Bootstrap lookup is not runtime directory authority.

Bootstrap authority is not runtime authority.

## Initial Directory Sprint Scope

Directory Sprint 1 should implement:

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

It should not implement distributed lookup, replication, caching, remote directory authorities, POSIX path emulation, complex mounts, or policy language.

## Summary

The Directory Service provides discoverability, not authority.

Its central rule is:

```text
Lookup is not access.
```
