# ATX-SPEC-007 Namespace Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This specification formalizes the Atarix Namespace Model.

The Namespace Model defines human-readable names, paths, bindings, aliases, and namespace structure.

The Namespace Model exists to make the system comprehensible without turning names into authority.

## Core Rule

```text
Lookup is not access.
```

Names help humans and services find objects.

Names do not grant authority.

## Name, Binding, Identity

The core relationship is:

```text
Name
  -> Binding
  -> Object identity
  -> Object
```

A name is not an object.

A binding is not a capability.

An object identity is not authority.

## Namespace Axioms

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Physical location is not identity.
```

## Namespace Layout

Recommended top-level namespaces:

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

These are architectural names. Their implementation may vary by platform maturity.

## `/system`

System-owned objects required for node or fabric operation.

System objects are not exempt from ownership, lifecycle, audit, cleanup, scope, or persistence rules.

## `/service`

Service objects and service interfaces.

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

## `/device`

Device objects and device-facing abstractions.

## `/fabric`

Fabric-visible nodes, links, memory pools, and fabric services.

Rev A assumes one operational fabric.

Future federated fabrics require explicit specification.

## `/user`

User-associated namespace roots.

User names are labels and administration handles, not authority.

## `/session`

Session-scoped objects and temporary runtime bindings.

Session entries are lease-oriented by default.

## `/temporary`

Temporary objects and bindings.

Temporary entries require explicit cleanup policy.

## `/audit`

Audit-facing objects and audit service interfaces.

Audit visibility is policy-controlled.

Audit visibility does not imply control authority.

## Binding Generation

Bindings should carry generation metadata.

Generation identifies binding evolution, not schema compatibility.

Versioning identifies schema or protocol compatibility.

These concepts must not be conflated.

## Aliases

Aliases provide alternate names.

Aliases must not create authority.

Aliases must not bypass ring, capability, lifecycle, resource, or policy checks.

Alias loops must be detected.

## Rebinding

A name may be rebound only by an authority permitted to modify that binding.

Rebinding must update generation metadata and create an audit event.

Clients requiring stability must pin object identity or binding generation rather than assuming name stability.

## Unnamed Objects

Objects may be unnamed.

Unnamed objects remain subject to authority, lifecycle, resource, audit, and cleanup rules.

## Namespace And Directory Service

The Namespace Model defines naming semantics.

The Directory Service implements name-to-object resolution.

Directory lookup provides discoverability, not authority.

## Namespace And Resources

Resources may be named, but resource visibility does not grant allocation or control authority.

## Namespace And Errors

Malformed paths, stale bindings, alias loops, unknown namespace authority, and unsupported versions are explicit errors.

Authority-bearing operations must fail closed when namespace state cannot be verified.

## Required Future Work

- Define exact path syntax.
- Define allowed character set.
- Define path and component length limits.
- Define canonical path rules.
- Define namespace policy interaction.
- Define federated namespace rules.
- Define version negotiation for namespace records in ATX-SPEC-012.

## Summary

The Namespace Model gives Atarix human-readable structure.

It does not grant authority.
