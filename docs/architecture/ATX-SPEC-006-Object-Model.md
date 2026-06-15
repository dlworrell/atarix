# ATX-SPEC-006 Object Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This specification formalizes the Atarix Object Model.

Objects are the fundamental architectural entities of Atarix.

Services, devices, resources, mailboxes, directory bindings, compatibility environments, virtual hardware, supervisor interfaces, and auxiliary compute providers are represented as objects or object-owned resources.

## Core Rule

```text
Name is not identity.
Identity is not authority.
```

An object may be named, discovered, observed, moved, virtualized, or persisted without changing its identity or authority model.

## Object Identity

An object's identity is:

- Opaque
- Immutable
- Never reused
- Independent of name
- Independent of physical location
- Independent of current provider
- Independent of capability possession

Object identity must not be a memory address, table slot, mailbox number, CPU-local handle, physical bus address, or implementation detail.

## Required Object Metadata

Every object has:

```text
Object identity
Object type
Owner or responsible authority
Lifecycle state
Ring association or security domain
Resource accounting
Audit visibility
Version or schema information
```

## Object Names

Objects may have zero, one, or many names.

Names are namespace bindings.

Names are useful for humans, administration, discovery, and service lookup.

Names are not identity and do not grant authority.

## Object Authority

Possessing an object identity does not grant authority.

Operating on an object requires explicit authority through capability, ownership policy, ring policy, lifecycle state, resource policy, and system policy.

## Object Lifecycle

Objects participate in the Lifecycle Model.

Objects may be active, suspended, revoked, expired, quarantined, recovering, destroyed, failed, or unknown depending on type.

Destroyed object identities must not be reused.

## Object Resources

Objects may own, allocate, reserve, expose, or consume resources.

Resource allocation is authority-bearing and must be explicit.

Object ownership does not automatically imply full authority over all resources associated with the object.

## Object Audit

Object events should be auditable, including:

```text
Object created
Object named
Object rebound
Object activated
Object suspended
Object granted authority
Object denied authority
Object quarantined
Object destroyed
Object recovered
```

Audit visibility does not imply control authority over the object.

## Object Errors

Unknown object state must be explicit.

If object identity, lifecycle state, version, policy, or authority cannot be verified, authority-bearing operations must fail closed.

## Services As Objects

A service is an object that offers one or more interfaces.

Examples:

```text
/service/audit
/service/storage
/service/network
/service/hash
/service/compression
/service/directory
```

Service location is not service identity.

The provider may be a CPU card, auxiliary compute card, storage processor, network card, or future fabric node.

## Virtual Hardware And Compatibility Objects

Virtual hardware, compatibility VMs, POSIX personalities, and legacy environments are objects.

They must have explicit capabilities, bounded resources, lifecycle policy, audit visibility, versioned interfaces, and cleanup.

Compatibility must not become ambient authority.

## Supervisor Objects

Supervisor-facing objects and supervisor audit exports are objects, but supervisor control remains isolated in the Supervisor Management Fabric.

Operational objects may observe supervisor state only through authorized observation or audit bridges.

Observation is not control.

## Local And Remote Objects

An object may be local, remote within the same operational fabric, or eventually remote through a federated fabric.

Rev A requires only one operational fabric.

Physical location must not be exposed as public object identity.

## Required Future Work

- Define object identity wire format.
- Define object type registry.
- Define object schema versioning in ATX-SPEC-012.
- Define service objects in ATX-SPEC-019.
- Define virtual hardware and POSIX compatibility in ATX-SPEC-015.
- Define storage-backed persistent objects in ATX-SPEC-017.

## Summary

Everything visible to Atarix is modeled as an object or object-owned resource.

Objects provide identity and lifecycle context, not automatic authority.
