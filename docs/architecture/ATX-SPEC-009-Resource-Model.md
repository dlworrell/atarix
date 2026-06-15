# ATX-SPEC-009 Resource Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Resource Model.

A resource is anything finite that an object may consume, reserve, control, expose, delegate, or depend upon.

Resource use is authority-bearing and must be explicit.

Resource visibility is not control.

Resource cleanup is a security requirement.

## Core Invariants

1. Every resource has an owner or allocator authority.
2. Every resource has lifecycle state.
3. Every resource has accounting metadata.
4. Every resource has a cleanup path.
5. Temporary resources use leases by default.
6. Persistent reservations require policy.
7. Visibility does not imply control.
8. Allocation does not bypass ring policy.
9. Ownership does not grant all operations.
10. Exhaustion must fail safely.
11. Cleanup must be auditable.
12. Leaks are architectural defects.
13. Resource identities must not expose private handles.
14. System resources are not exempt.

## Resource Classes

Conceptual resource classes include:

```text
Compute resources
Memory resources
Communication resources
Storage resources
Device resources
Fabric resources
Supervisor resources
Environmental resources
Auxiliary compute resources
```

Auxiliary compute resources include service-provider cards such as Raspberry Pi-class compute/buffer cards, RISC-V service cards, GPU service cards, or storage processor cards.

They provide services and resource pools, not automatic native CPU authority.

## Resource Identity And Naming

Resources may be published through the Directory Service.

Examples:

```text
/device/network/eth0
/fabric/node/0/cpu/0
/fabric/node/0/memory/local
/service/hash
/service/compression
/system/resource-manager
```

Resource names are not resource identities.

Resource identities are not authority.

Resource lookup does not grant allocation authority.

## Resource Ownership

Ownership answers who is responsible for a resource.

Ownership does not automatically answer what operations are allowed.

Allocation, observation, accounting, revocation, and control are separate authorities.

## Allocation Records

A resource allocation must include:

```text
Allocation ID
Resource identity or pool identity
Owner
Requester
Authority used
Scope
Lifecycle state
Creation time
Lease or persistence policy
Quota impact
Cleanup policy
Audit policy
Version or schema identifier
```

## Leases And Quotas

Temporary allocations use leases by default.

Lease expiration must trigger reclamation, revocation, or quarantine.

Quota enforcement must fail safely.

If quota state cannot be verified, new authority-bearing allocations should be denied.

## Revocation And Reclamation

Revocation may be immediate, deferred, graceful, forced, quarantined, unsupported, or policy-denied.

Resources must be reclaimed when:

```text
Owner exits
Owner is destroyed
Lease expires
Capability is revoked
Object is uninstalled
Session ends
Node crashes
Fabric partitions
Policy changes
Quota enforcement requires it
```

Reclamation may destroy, return, quarantine, scrub, or transfer the resource by explicit policy.

## Resource Exhaustion

Resource exhaustion is expected and must be safe.

Atarix must not grant broader authority as a fallback for exhaustion.

Audit, supervisor, and recovery resources should be protected from normal workload exhaustion.

## Resource Visibility

Authorized observers may inspect resource state.

Observation may include identity, type, owner, allocation state, quota use, lease expiration, lifecycle state, audit state, and health state.

Observation does not imply control.

## Resource And Capability Interaction

Capabilities may authorize operations such as:

```text
READ
WRITE
CONTROL
ALLOCATE
RESERVE
DELEGATE
REVOKE
OBSERVE
ACCOUNT
```

Capabilities remain constrained by ring policy, lifecycle state, expiration, revocation, and policy.

## Supervisor Resources

Supervisor resources belong to the Supervisor Management Fabric, not the Operational Fabric.

Operational objects may observe supervisor state through authorized audit or observation bridges, but may not directly control supervisor resources.

Examples:

```text
Reset lines
Power rails
Watchdog timers
RTC state
Firmware validation state
Recovery mode state
Fault logs
```

## Memory Pooling Rule

Resources are not pooled merely because they exist.

Resources are pooled only when ownership, authority, failure domain, and recovery semantics are compatible.

CPU-local SRAM, fabric RAM, auxiliary compute RAM, RAM cards, and supervisor RAM are distinct resource domains.

Supervisor memory is not operational scratch space.

Auxiliary compute RAM may be exposed as a capability-mediated fabric resource pool, not transparent CPU-local RAM.

## Audit Buffering Target

Long-term audit architecture may use staged buffers:

```text
Supervisor audit buffer
Operational RAM audit buffer
Fabric RAM audit journal
Persistent checksummed storage
Archive or replication target
```

This is a target architecture and not a Rev A bootstrap dependency.

## Bootstrap Resources

Bootstrap resources belong to the Bootstrap Security Model.

Bootstrap resource authority is not runtime resource authority.

## Initial Resource Sprint Scope

Resource Sprint 1 should implement:

```text
Resource type identifiers
Resource object metadata
Ownership metadata
Allocation records
Release records
Lease expiration semantics
Accounting counters
Basic tests
```

## Summary

Atarix resources are architectural entities subject to authority, lifecycle, audit, cleanup, and policy.

The central rules are:

```text
Resource use is authority-bearing and must be explicit.
Observation is not control.
```
