# ATX-SPEC-002 Authority Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines authority within Atarix.

Authority is the right to perform an operation on or through an object, resource, service, namespace binding, supervisor function, or fabric facility.

Authority is explicit. Authority is scoped. Authority is auditable. Authority is revocable where possible.

## Core Rule

```text
Identity is not authority.
```

Authority must never be inferred from user identity, object identity, physical location, execution state, network reachability, directory visibility, or implementation detail.

## Authority Sources

Recognized authority sources include:

```text
Object ownership policy
Explicit capabilities
Delegated capabilities
Ring policy
Resource policy
Lifecycle policy
Supervisor-mediated authority
Bootstrap authority
Administrative policy
Recovery policy
```

No source is ambient. Each source must be visible to audit and policy review.

## Authority Domains

### Runtime Authority

Normal authority used by runtime objects, services, applications, drivers, and fabric participants.

Runtime authority is mediated by ring policy, capabilities, lifecycle state, resource policy, and object policy.

### Bootstrap Authority

Narrow authority available before the runtime authority model exists.

Bootstrap authority may initialize minimal hardware, validate firmware, fetch boot data, record boot events, and enter recovery.

Bootstrap authority is not runtime authority and must not silently persist into runtime.

### Supervisor Authority

Authority belonging to the Supervisor Management Fabric.

Supervisor authority controls reset, watchdog, RTC, power, firmware validation, recovery, and fault reporting.

The Operational Fabric may request supervisor action through mediated interfaces but may not directly become supervisor.

### Recovery Authority

Authority used to restore, quarantine, reconcile, or repair system state.

Recovery authority must be explicit, auditable, and constrained by policy.

Recovery must not silently regrant authority.

### Administrative Authority

Authority granted to administrative users, services, or policy domains.

Administrative identity is not itself authority; administrative authority must still be represented by policy and capability state.

## Ownership

Ownership answers:

```text
Who is responsible for this object or resource?
```

Ownership does not automatically answer:

```text
What operations may be performed?
```

Ownership may influence authority but does not replace capabilities, policy, or ring checks.

## Delegation

Delegation transfers or derives authority from an existing authority.

Delegation must:

- Never increase authority beyond the delegator's authority.
- Preserve auditability.
- Preserve scope.
- Preserve lifecycle constraints.
- Preserve resource constraints.
- Preserve ring constraints.
- Be revocable where supported.

Delegation across ring boundaries requires explicit policy.

## Revocation

Revocation removes or invalidates authority.

Revocation may be immediate, deferred, graceful, forced, quarantined, unsupported, or policy-denied depending on the object and resource type.

Revocation must be auditable.

Revocation of authority may require resource cleanup, mailbox closure, directory update, lifecycle transition, or quarantine.

## Authority And Directory

Directory lookup never grants authority.

Directory binding ownership controls name lifecycle.

Object ownership controls object responsibility.

Capability or policy state controls operation authority.

These must not be conflated.

## Authority And Resources

Resource allocation is authority-bearing.

Resource ownership is accountability, not unrestricted control.

Allocation authority, observation authority, accounting authority, revocation authority, and control authority are separate.

## Authority And Audit

Every authority-bearing operation should identify:

```text
Actor identity label
Authority used
Operation
Target object or resource
Policy result
Result
Reason
```

Audit visibility does not imply authority over the audited object.

## Authority And Errors

If authority cannot be verified, the operation must fail closed.

If authority state is unknown after crash, partition, version mismatch, or recovery, affected objects or resources must be restricted or quarantined until reconciled.

## Compatibility Authority

Future POSIX or virtual hardware compatibility environments must receive explicit bounded authority.

Compatibility environments may translate legacy interfaces, but they may not import ambient authority into native Atarix.

## Required Future Work

- Define authority object or authority-record wire format.
- Define ownership transfer semantics.
- Define shared administrative sponsorship.
- Define bootstrap authority in ATX-SPEC-014.
- Define supervisor authority in ATX-SPEC-016.
- Define policy evaluation in ATX-SPEC-013.

## Summary

Authority is not identity, not lookup, not observation, not execution, and not connectivity.

Authority is explicit, bounded, auditable, and policy-mediated.
