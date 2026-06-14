# Atarix Namespace Model

## Purpose

The Namespace Model defines how human-readable names resolve to Atarix objects.

Users and services interact with names.

The kernel and fabric operate on objects.

Hardware operates on physical locations.

These concepts must remain separate.

## Design Goals

The namespace system shall:

1. Support human-readable names.
2. Support object relocation.
3. Support service upgrades.
4. Support distributed fabrics.
5. Support capability-mediated access.
6. Never expose physical addresses as public identifiers.
7. Never require object identifiers to be human readable.
8. Never treat lookup as authorization.

## Identity Hierarchy

Atarix distinguishes four concepts.

### Physical Location

Examples:

```text
Node 17
Mailbox 42
CPU 0
Memory 0x12340000
```

Physical location is not stable and is not user visible.

### Object Identity

Example:

```text
OID 6dfe4b34-8b41-4a31-9a49-a73a8a36d251
```

Object identity is immutable, opaque, globally unique within the fabric, and never reused.

### Name

Example:

```text
storage.boot
```

A name is human readable and mutable.

Names are bindings, not identities.

### Path

Example:

```text
/services/storage.boot
/fabric/node17/storage.boot
```

A path resolves to a namespace binding.

## Core Rule

Names are not objects.

Objects are not names.

Correct model:

```text
name
  -> binding
  -> OID
  -> object
```

## Name Bindings

A namespace entry binds a human-readable name or path to an object identity.

Conceptually:

```c
typedef struct
{
    char name[64];
    atx_oid_t target;
} atx_name_binding_t;
```

The namespace contains bindings.

The namespace does not contain objects.

## Lookup Process

A typical lookup proceeds as follows:

```text
name
  -> binding
  -> OID
  -> object resolution
  -> ring check
  -> capability check
  -> usable object reference
```

The caller shall never receive raw pointers or physical addresses.

## Aliases

Multiple names may resolve to the same object identity.

Example:

```text
/services/storage.boot
/services/boot-storage
/services/primary-storage
```

all may resolve to the same OID.

Aliases are legal.

Aliases do not create new objects.

## Object Replacement

Names may be rebound to different objects.

Example:

```text
/services/storage.boot -> OID A
```

may later become:

```text
/services/storage.boot -> OID B
```

This supports service replacement and upgrades.

The name changes binding.

Neither object changes identity.

## Unnamed Objects

Objects may exist without namespace entries.

Examples include:

- Temporary mailboxes
- Capability objects
- Kernel IPC endpoints
- Shared memory regions
- Quarantined objects

Unnamed objects retain identity and may still be referenced internally by OID or capability.

## Namespace Layout

The initial namespace should use a virtual filesystem-style hierarchy.

Recommended top-level namespaces:

```text
/
  services/
  nodes/
  users/
  devices/
  system/
  fabric/
```

### Services

```text
/services/storage.boot
/services/logger
/services/discovery
/services/scheduler
```

### Nodes

```text
/nodes/node0
/nodes/node1
/nodes/node2
```

### Devices

```text
/devices/disk0
/devices/net0
/devices/display0
```

### System

Kernel-owned namespace:

```text
/system/ring-manager
/system/capability-manager
/system/object-manager
/system/directory
```

## Fabric Scope

The same resolution model applies locally and across the fabric.

Examples:

```text
/services/storage.boot
/nodes/node17/services/storage.boot
/fabric/services/storage.boot
```

Only scope changes.

The distinction between local and remote objects shall not leak into public naming rules.

## Security Rule

Lookup success does not imply access.

Knowing that an object exists does not grant authority to use it.

Access still requires:

- Ring validation
- Capability validation
- Object state validation

Directory lookup and authorization are separate operations.

## Ring Interaction

Ring restrictions apply after name resolution.

Example:

```text
User ring caller
  -> lookup /system/scheduler
  -> OID found
  -> ring check
  -> denied unless explicitly mediated
```

Object existence is not permission.

## Name Lifetime

Names may be:

- Created
- Renamed
- Moved
- Deleted
- Aliased
- Rebound

Object identities remain unchanged.

## Invariants

The following statements must always remain true:

1. Names are bindings.
2. Names are not identities.
3. Objects are not names.
4. Objects may be unnamed.
5. Multiple names may bind to one object.
6. A name may be rebound to another object.
7. Lookup does not imply access.
8. Physical location is never exposed as a public namespace identity.
9. Local and remote lookup must follow the same conceptual model.
