# ATX-SPEC-003 Capability Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines capabilities in Atarix.

A capability is an explicit authority-bearing object or record that permits specific operations against a specific object, service, resource, namespace binding, mailbox endpoint, or controlled interface.

Capabilities are not identities.

Capabilities are not names.

Capabilities are not directory entries.

## Core Rule

```text
Possession of identity is not possession of authority.
```

A caller may know an object exists and still lack a capability to operate on it.

## Capability Properties

A capability must be:

- Explicit
- Scoped
- Auditable
- Revocable where possible
- Bound to intended authority
- Constrained by ring policy
- Constrained by lifecycle state
- Constrained by resource policy
- Constrained by expiration or lease where applicable

## Capability Scope

A capability may be scoped by:

```text
Object identity
Resource identity or pool
Directory binding
Mailbox endpoint
Service interface
Operation set
Ring
Owner
Session
Time
Node
Fabric
Policy domain
```

## Capability Operations

Common operations include:

```text
READ
WRITE
CONTROL
ALLOCATE
RESERVE
OBSERVE
ACCOUNT
DELEGATE
REVOKE
CREATE
DESTROY
EXECUTE
RECOVER
```

Operation names are architectural names. Their wire-format encodings are implementation details.

## Capability And Ring Interaction

Rings provide coarse-grained security boundaries.

Capabilities provide fine-grained authority within or across those boundaries.

A capability does not automatically bypass ring policy.

A ring level does not automatically grant a capability.

Cross-ring use must be explicit and auditable.

## Capability And Directory Interaction

Directory lookup does not return authority.

A directory entry may help locate an object, service, or resource, but the caller must still present or derive a valid capability before performing authority-bearing operations.

## Capability And Resource Interaction

Resource use is authority-bearing.

Capabilities may authorize resource allocation, reservation, observation, accounting, revocation, or control.

Resource visibility does not grant resource control.

## Delegation

Delegation creates a derived authority from an existing authority.

Delegated capabilities must not exceed the authority of the delegator.

Delegation must preserve:

```text
Scope
Operation limits
Ring limits
Resource limits
Lifecycle limits
Expiration
Audit chain
Revocation relationship
```

## Revocation

Revocation invalidates capability authority.

Revocation may require:

```text
Mailbox closure
Resource reclamation
Directory update
Lifecycle transition
Audit event
Quarantine
```

A revoked capability must remain denied after ring checks, resource checks, or directory resolution.

## Expiration And Leases

Capabilities may be time-bounded.

Expiration must not silently convert into persistence.

Expired capabilities must fail closed for authority-bearing operations.

## Capability And Audit

Capability events must be auditable, including:

```text
Capability created
Capability granted
Capability delegated
Capability used
Capability denied
Capability expired
Capability revoked
Capability destroyed
```

Audit records should identify the authority chain used for the decision.

## Capability And Error Handling

If capability validity cannot be verified, the operation must fail closed.

If capability state is unknown after crash or recovery, affected authority should be quarantined or denied until reconciled.

## Compatibility Environments

Compatibility VMs, POSIX personalities, virtual hardware, auxiliary compute providers, and service shims are objects and require explicit capabilities.

Compatibility must not become an ambient authority bridge.

## Required Future Work

- Define concrete capability wire format.
- Define capability identity versus capability record identity.
- Define revocation graph semantics.
- Define capability serialization rules.
- Define compatibility with future Policy Model.
- Define version negotiation for capability records in ATX-SPEC-012.

## Summary

Capabilities are explicit authority.

Knowing, naming, observing, or executing is not enough.
