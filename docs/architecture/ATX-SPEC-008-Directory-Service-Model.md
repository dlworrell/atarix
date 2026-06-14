# ATX-SPEC-008 Directory Service Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Directory Service Model.

The Directory Service maps human-readable names to object identities. It provides discovery, binding, aliasing, enumeration, and lookup semantics for Atarix objects.

The Directory Service does not grant authority.

Lookup is not access.

Knowing a name is not authority.

Resolving an object identity is not authority.

All operations on resolved objects remain subject to ring policy, ownership rules, capability validation, lifecycle state, and resource policy.

## Design Philosophy

Atarix separates:

- Name
- Binding
- Object identity
- Object location
- Authority
- Capability
- Lifecycle state

The Directory Service exists to answer:

```text
Given this name, what object identity does it currently bind to?
```

It does not answer:

```text
May this caller use the object?
```

That question belongs to the Security, Authority, Capability, Policy, and Lifecycle models.

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
10. Bindings can become stale.
11. Stale bindings must be observable.
12. Stale bindings must be removable.
13. Destroyed object identities must never be reused to satisfy stale bindings.
14. Directory operations must be auditable.
15. Directory failure must fail closed for authority-bearing operations.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model

This specification informs:

- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- Future POSIX Compatibility Model

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

### Path

A hierarchical name composed of path components.

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

### `/system`

System-owned objects required for node or fabric operation.

Examples:

```text
/system/object-manager
/system/directory
/system/security
/system/lifecycle
/system/policy
```

System objects are not exempt from ownership, audit, lifecycle, cleanup, scope, or persistence rules.

### `/service`

Longer-lived service objects.

Examples:

```text
/service/discovery
/service/audit
/service/network
/service/storage
```

### `/device`

Device objects and device-facing service abstractions.

Examples:

```text
/device/network/eth0
/device/storage/boot0
/device/serial/console0
```

### `/fabric`

Fabric-visible nodes, links, and fabric services.

Examples:

```text
/fabric/node/0
/fabric/node/0/cpu/0
/fabric/node/0/memory/local
/fabric/link/0
```

### `/user`

User-associated namespace roots.

Example:

```text
/user/donovan
```

User names are compatibility and administration labels. They are not authority by themselves.

### `/session`

Session-scoped objects and temporary bindings.

Example:

```text
/session/active/0
```

Session bindings are lease-oriented by default.

### `/temporary`

Temporary objects, scratch bindings, leases, and short-lived resources.

Objects in this subtree must have explicit lifetime and cleanup policy.

### `/audit`

Audit-visible objects and audit service interfaces.

This namespace does not necessarily expose raw audit records. Visibility is policy-controlled.

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
```

A binding may additionally contain:

```text
Alias target
Object type hint
Preferred display name
Version information
Node hint
Cache hint
Replication policy
Policy reference
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

### DIRECT

A direct name-to-object identity binding.

### ALIAS

A binding that resolves to another name or canonical path.

Aliases must not create authority.

### MOUNT

A binding that connects one namespace subtree to another namespace authority or directory object.

Mounts require explicit authority and audit.

### REDIRECT

A binding that redirects lookup to another directory service or fabric node.

Redirects require loop detection and policy validation.

### TOMBSTONE

A record that intentionally preserves knowledge that a binding used to exist but has been removed.

Tombstones are useful for audit and stale-reference detection.

### STALE

A binding whose target can no longer be verified.

Stale bindings must not silently resolve to replacement objects.

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
```

Lookup does not return a capability.

Lookup does not grant object access.

Lookup does not imply that future lookup of the same name resolves to the same object unless a binding generation or identity pin is used.

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
```

Failures must map into the Error Model when ATX-SPEC-011 is defined.

Until then, failure categories should remain explicit and distinguishable in tests.

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

Examples:

```text
A user may know that /service/print exists.
That does not imply permission to submit print jobs.

A node may know that /fabric/node/7 exists.
That does not imply permission to communicate with it.
```

## Enumeration Semantics

Directory enumeration lists entries within a namespace subtree.

Enumeration may return:

```text
Name
Binding type
Object type hint
Lifecycle state
Generation
Visibility metadata
```

Enumeration must not return capabilities.

Enumeration must not expose implementation-private physical addresses, table slots, raw mailbox numbers, or CPU-local pointers.

## Alias Semantics

Aliases provide alternate human-readable names.

Example:

```text
/system/log
  -> /service/audit/logger
```

Alias resolution must:

- Detect loops.
- Preserve auditability.
- Preserve policy boundaries.
- Never create authority.
- Never bypass ring checks.
- Never bypass capability checks.

## Canonical Names

An object may have a canonical path for display, audit, and administrative purposes.

Canonical names are for human comprehension and operational stability.

Canonical names are not object identity.

An object's identity must remain stable if its canonical path changes.

## Name Rebinding

A name may be rebound to a different object only by an authority permitted to modify that binding.

Rebinding must update generation metadata and create an audit event.

Clients that require stability must pin object identity or binding generation rather than assuming names are stable.

## Binding Ownership

Bindings have owners.

The binding owner may differ from the target object owner.

For example:

```text
/service/print
```

may be owned by the system service manager while the underlying print object is owned by a print service authority.

Binding ownership controls name lifecycle.

Object ownership controls object authority.

These must not be conflated.

## Lifecycle Integration

Directory entries participate in the lifecycle model.

Bindings may be:

```text
CREATED
ACTIVE
SUSPENDED
STALE
EXPIRED
REMOVED
TOMBSTONED
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

## Temporary Bindings

Temporary bindings are lease-oriented by default.

Examples:

```text
/session/active/0/socket/3
/temporary/build/42
/temporary/discovery/node-probe
```

Temporary bindings must specify:

```text
Owner
Lease duration
Renewal authority
Expiration behavior
Cleanup authority
Audit policy
```

## Persistence Rules

Persistent bindings must be explicitly requested and justified.

Persistent binding metadata must include:

```text
Owner
Persistence policy
Recovery behavior
Cleanup authority
Audit visibility
```

Persistence must not be accidental.

## Distributed Directory Behavior

The Directory Service must support future distributed operation.

A directory lookup may resolve to an object on:

```text
The local node
Another node in the same fabric
A migrated object
A replicated service
A remote namespace authority
```

Distributed lookup must not weaken authority rules.

Remote resolution does not imply remote access.

## Caching

Directory lookup results may be cached.

Caches must respect:

```text
Binding generation
Lease / expiration
Revocation
Tombstone state
Namespace authority
Policy changes
```

A stale cache entry must not create authority.

Cache failure must fail closed for authority-bearing operations.

## Object Mobility

Objects may move between nodes or backing implementations.

Object mobility must preserve object identity unless the move creates a new object.

Directory bindings may update location hints, but object identity remains the stable reference.

Clients must not depend on physical location for identity.

## Security Requirements

The Directory Service must satisfy the following:

- Lookup is not authority.
- Enumeration is not authority.
- Aliases do not grant authority.
- Rebinding requires explicit authority.
- Mounting requires explicit authority.
- Redirects require policy validation.
- Stale bindings must not resolve to replacement objects.
- Directory failure must not create authority.
- Directory metadata must not expose implementation-private handles as public contracts.
- Directory state must be auditable.

## Attack Surface Requirements

The Directory Service is security-sensitive because it influences discoverability.

It must minimize attack surface by default:

- No unauthenticated mutation interfaces.
- No default broad enumeration of sensitive namespaces.
- No raw internal pointer exposure.
- No automatic trust of remote directory authorities.
- No silent fallback to insecure lookup.
- No debug mutation endpoints in production profiles.

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

Audit records should capture:

```text
Actor identity label
Authority used
Operation
Path
Binding ID
Target object identity
Generation
Result
Reason
Timestamp source
```

The Audit Model will define exact record format.

## Failure Rules

When directory state is uncertain, Atarix must preserve security over convenience.

If the Directory Service cannot verify a binding for an authority-bearing operation, the operation must fail closed.

If a lookup is advisory and not authority-bearing, policy may allow degraded operation, but degraded operation must be explicit and auditable.

## Bootstrap Considerations

Some early boot services may need name-like resolution before the full Directory Service exists.

That behavior belongs to the Bootstrap Security Model.

Bootstrap lookup must not be treated as normal runtime directory authority.

Bootstrap authority is not runtime authority.

## POSIX Compatibility Considerations

Future POSIX compatibility will map POSIX paths onto Atarix namespace and directory operations.

POSIX path lookup must not import POSIX ambient-authority assumptions into Atarix.

A POSIX file descriptor should eventually wrap Atarix object identity and capability state rather than becoming native authority by itself.

## Initial Directory Sprint Scope

Directory Sprint 1 should implement only the smallest useful subset:

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

Directory Sprint 1 should not implement:

```text
Distributed lookup
Replication
Caching
Remote directory authorities
POSIX path emulation
Complex mounts
Policy language
```

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

Atarix names help humans and services find objects, but capabilities and policy still decide what operations are allowed.
