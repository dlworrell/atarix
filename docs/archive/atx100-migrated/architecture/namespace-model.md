# Atarix Namespace Model

## Purpose

The Namespace Model defines how human-readable names resolve to Atarix objects.

Users and services interact with names. The kernel and fabric operate on objects. Hardware operates on physical locations. These concepts must remain separate.

## Core Rule

Names are not objects.

Objects are not names.

Correct model:

```text
name
  -> binding
  -> object identity
  -> object
```

## Identity Hierarchy

Atarix distinguishes:

1. Physical location
2. Object identity
3. Name
4. Path

Physical location is not stable and is not user visible.

Object identity is opaque, immutable, globally unique within the fabric, and never reused.

A name is human-readable and mutable.

## Lookup Rule

Lookup success does not imply access.

Knowing that an object exists does not grant authority to use it.

Access still requires ring validation, capability validation, and object-state validation.

## Namespace Layout

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

## Invariants

1. Names are bindings.
2. Names are not identities.
3. Objects may be unnamed.
4. Multiple names may bind to one object.
5. A name may be rebound to another object.
6. Lookup does not imply access.
7. Physical location is never exposed as public namespace identity.
