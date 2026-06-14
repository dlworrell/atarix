# Atarix Object Model

## Purpose

The Object Model defines the fundamental unit of identity, authority, naming, ownership, communication, and resource management within Atarix.

All externally visible entities within Atarix are represented as objects.

Examples include:

- Kernel services
- Mailboxes
- Devices
- Storage entities
- Fabric nodes
- User services
- Future processes and tasks

This document defines the common properties shared by all objects.

## Definition

An object is a uniquely identifiable entity that participates in the Atarix namespace and may possess ownership, authority, capabilities, resources, and communication endpoints.

Objects are the primary architectural abstraction of the system.

## Object Identity

Identity is the fundamental property of an object.

An object's identity:

- Exists independently of its name.
- Exists independently of its location.
- Exists independently of its capabilities.
- Exists independently of its communication endpoints.
- Is opaque.
- Is immutable.
- Is globally unique within the Atarix fabric.
- Is never reused.

Identity shall never be inferred from physical location, ring number, table slot, local handle, memory address, or implementation-specific layout.

Names may change.

Locations may change.

Identity does not change.

## Object Identifier Format

The preferred object identity format is a 128-bit opaque identifier.

The internal representation should be treated as bytes, not as a structured integer with semantic fields.

Conceptually:

```c
typedef struct
{
    uint8_t bytes[16];
} atx_oid_t;
```

External rendering may use a UUID-like textual form, but the text form is only presentation.

Code shall not parse object identifiers to infer node, ring, type, owner, or physical location.

## Object Naming

Objects may possess one or more names.

Names are human-readable references used to locate an object.

Examples:

```text
kernel.directory
kernel.scheduler
service.discovery
device.serial0
storage.boot
fabric.node.7
```

Names are not identity.

Multiple names may refer to the same object.

An object may exist even when no public name is assigned.

Naming behavior is further defined by the Namespace Model.

## Object Types

Every object possesses a type.

Initial object types include:

```text
Kernel
Service
Mailbox
Device
Storage
Fabric
User
```

Additional object types may be introduced through future architectural review.

## Ownership

Every object possesses an owner.

Ownership determines:

- Creation authority
- Delegation authority
- Destruction authority
- Resource accountability

Ownership may be transferred only through mechanisms defined by the Authority Model.

## Authority

Objects do not automatically possess authority over other objects.

Authority is granted through:

- Ownership
- Capabilities
- Explicit delegation

Authority must be traceable and auditable.

No operation shall succeed through undocumented special-case behavior.

## Ring Association

Every object is associated with exactly one ring.

Examples:

```text
Kernel
Supervisor
User
Quarantine
```

Ring association establishes coarse-grained security boundaries.

Ring association does not replace capability checks.

Ring behavior is defined by the Security Model.

## Lifecycle

Every object participates in a lifecycle.

Minimum lifecycle states:

```text
Created
Initialized
Active
Suspended
Destroyed
```

Additional states may exist for specific object types.

Destroyed objects shall never re-enter an active state.

## Resources

Objects may own resources.

Examples include:

- Memory
- Mailboxes
- Capabilities
- Storage
- Fabric routes
- Processing time

Ownership of resources implies responsibility for resource reclamation.

Resource accounting is defined by the Resource Model.

## Communication

Objects communicate through architectural mechanisms such as mailboxes.

Communication endpoints are not identity.

Communication endpoints are resources owned by objects.

Objects may possess:

- Zero endpoints
- One endpoint
- Multiple endpoints

## Relationships

Objects may maintain relationships with other objects.

Examples:

```text
Parent
Child
Owner
Delegate
Peer
Supervisor
```

Relationships do not alter object identity.

## Local and Remote Objects

Objects may exist locally or remotely.

Remote objects shall be treated as architectural peers of local objects whenever practical.

Subsystems should avoid assumptions that require all objects to exist on the local node.

Location is an attribute of an object, not its identity.

## Audit Requirements

Operations affecting an object should be auditable.

Examples:

- Creation
- Destruction
- Delegation
- Revocation
- Ownership transfer
- Capability issuance

Audit behavior is defined by the Audit Model.

## Invariants

The following statements must always remain true:

1. Every object possesses exactly one identity.
2. Identity is opaque.
3. Identity is immutable.
4. Identity is globally unique within the Atarix fabric.
5. Identity is never reused.
6. Identity is not a name.
7. Identity is not a mailbox.
8. Identity is not a capability.
9. Identity is not a memory address.
10. Identity is not a table slot, index, or local handle.
11. Every object possesses exactly one owner.
12. Every object belongs to exactly one ring.
13. Destroyed objects cannot become active again.
14. Authority must be traceable.
15. Objects may exist without names.

## Open Questions

Q-0001:
Should object identities remain globally unique beyond a single fabric, such as across exported archives or federated fabrics?

Q-0002:
Can ownership be shared among multiple objects, or must shared control be modeled as delegated authority?

Q-0003:
Should aliases be first-class namespace entities or merely alternate name bindings?
