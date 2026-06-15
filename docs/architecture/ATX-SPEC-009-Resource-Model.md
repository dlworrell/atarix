# ATX-SPEC-009 Resource Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Resource Model.

A resource is anything finite that an object may consume, reserve, control, expose, delegate, or depend upon.

Resource use is authority-bearing and must be explicit.

Resource visibility is not authority.

Resource allocation is not ownership.

Resource cleanup is a security requirement.

## Design Philosophy

Atarix treats resources as first-class architectural entities rather than incidental implementation details.

A resource must be:

- Named or discoverable where appropriate.
- Owned or attributable.
- Accounted.
- Constrained.
- Auditable.
- Revocable where possible.
- Reclaimed or quarantined when no longer valid.

The Resource Model answers:

```text
Who owns this resource?
Who may allocate it?
Who is consuming it?
How long may it live?
What authority keeps it alive?
What happens when it is revoked?
What happens when the owner disappears?
```

## Core Invariants

1. Every resource has an owner or allocator authority.
2. Every resource has a lifecycle state.
3. Every resource has accounting metadata.
4. Every resource has a cleanup path.
5. Temporary resources use leases by default.
6. Persistent resource reservations require explicit policy.
7. Resource visibility does not imply control.
8. Resource allocation does not bypass ring policy.
9. Resource ownership does not automatically grant all operations on the resource.
10. Resource exhaustion must fail safely.
11. Resource cleanup must be auditable.
12. Resource leaks are architectural defects.
13. Resource identities must not expose implementation-private handles as public contracts.
14. Resource state must be inspectable by authorized observers.
15. System resources are not exempt from lifecycle, audit, or cleanup rules.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model
- Future scheduler, network, filesystem, process, and POSIX compatibility specifications

## Resource Classes

Atarix recognizes these conceptual resource classes:

```text
Compute resources
Memory resources
Communication resources
Storage resources
Device resources
Fabric resources
Supervisor resources
Environmental resources
Audit resources
Recovery resources
```

## Compute Resources

Examples:

```text
CPU core
CPU time slice
scheduler queue slot
accelerator context
GPU compute queue
FPGA region
auxiliary compute card service
```

A compute resource may be provided by a CPU card, auxiliary compute card, GPU card, FPGA region, Raspberry Pi-class service card, RISC-V service card, storage processor, or future provider.

CPU capability is not system capability.

A weak primary CPU may still access strong system services through explicit resource and service interfaces.

## Memory Resources

Examples:

```text
CPU-local SRAM
FPGA-local buffers
fabric memory
RAM-card memory
DMA buffer
shared memory region
page frame
kernel heap allocation
audit buffer pool
```

Memory location is not memory authority.

CPU-local SRAM, fabric RAM, RAM-card memory, and supervisor RAM have different authority and failure semantics.

Resources are not pooled merely because they exist.

Resources are pooled only when ownership, authority, failure domains, and recovery semantics are compatible.

## Supervisor Resources

Supervisor resources belong to the Supervisor Management Fabric, not the Operational Fabric.

Examples:

```text
reset line
watchdog
RTC
power rail control
firmware update slot
recovery mode control
health sensor
fault log
supervisor RAM
```

Operational objects may observe supervisor resource state through authorized observation or audit paths, but they must not directly control supervisor resources.

Observation is not control.

Supervisor memory is not operational scratch space.

## Audit Resources

Audit resources include:

```text
Audit ingress queues
High-speed audit buffers
RAM-backed audit journals
Audit persistence queues
Persistent audit storage
Supervisor audit buffers
```

Audit resources should be reserved so normal workload exhaustion does not silently disable audit.

Audit generation should not require synchronous physical storage writes.

RAM-backed audit buffering is a target architecture once fabric RAM cards or compatible memory services are available.

## Resource Identity And Naming

Resources may be named through the Directory Service.

Examples:

```text
/device/network/eth0
/fabric/node/0/cpu/0
/fabric/node/0/memory/local
/fabric/node/0/memory/card0
/system/resource-manager
/system/power/budget
```

Resource names are not resource identities.

Resource identities are not authority.

Resource directory lookup does not grant resource access.

## Resource Ownership

Ownership answers:

```text
Who is responsible for this resource?
```

Ownership does not automatically answer:

```text
What operations are allowed on this resource?
```

Authority is still capability and policy mediated.

## Resource Allocation

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
Version or schema information
```

Allocation may be exclusive, shared, read-only, write-capable, control-capable, time-sliced, quota-limited, best-effort, reserved, or borrowed.

## Leases And Quotas

Temporary resource allocations use leases by default.

Quota enforcement must fail safely.

If quota state cannot be verified, new authority-bearing allocations should be denied.

Lease expiration must trigger reclamation, revocation, or quarantine.

## Revocation And Reclamation

Resource revocation may be:

```text
Immediate
Deferred
Graceful
Forced
Quarantined
Policy-denied
Unsupported
```

Resources must be reclaimed when the owner exits, owner is destroyed, lease expires, capability is revoked, session ends, node crashes, fabric partitions, policy changes, or quota enforcement requires it.

Reclamation may destroy, return, quarantine, scrub, or transfer the resource by explicit policy.

## Resource Exhaustion

Resource exhaustion is expected and must be safe.

When resources are exhausted, Atarix must:

- Fail closed for authority-bearing operations.
- Preserve system-critical recovery paths where possible.
- Preserve supervisor functions where possible.
- Preserve audit visibility where possible.
- Avoid granting broader authority as a fallback.
- Audit denied or degraded allocations.
- Prefer controlled degradation over undefined behavior.

## Resource Visibility

Authorized observers may inspect resource state.

Observation may include resource identity, type, owner, allocation state, quota use, lease expiration, lifecycle state, audit state, and health state.

Observation does not imply control.

## Resource And Capability Interaction

Capabilities grant explicit authority over resources.

A resource capability may authorize operations such as:

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

## Auxiliary Compute And Buffer Cards

Auxiliary compute cards are resource providers, not primary CPU cards.

Examples:

```text
Raspberry Pi 5-class compute/buffer card
ARM SBC card
RISC-V service card
GPU service card
AI accelerator card
Storage processor card
```

These cards may provide services such as hashing, compression, encryption, audit staging, network offload, netboot caching, storage staging, or ZFS-style scrub assistance.

Their RAM is exposed as explicit fabric resources, not as transparent CPU-local RAM.

## Crash And Recovery

After crash or reboot, the Resource Model must reconcile:

```text
Persistent allocations
Leased allocations
Expired leases
Outstanding reservations
Shared memory state
DMA state
Mailbox state
Directory bindings
Audit state
Owner state
```

Unknown resource state must be quarantined or denied until reconciled.

Recovery must not silently regrant authority.

## Bootstrap Resource Considerations

Some resources exist before the full runtime Resource Manager exists.

Examples:

```text
Boot ROM storage
Supervisor firmware image
Fabric bitstream image
Boot network endpoint
NTP bootstrap time source
Recovery image slot
```

Bootstrap resources belong to the Bootstrap Security Model.

Bootstrap resource authority is not runtime resource authority.

## POSIX Compatibility Considerations

Future POSIX compatibility will map POSIX resources onto Atarix resources.

Examples:

```text
File descriptor -> capability-wrapped resource reference
Process -> application/session object
Socket -> network endpoint resource
Pipe -> mailbox or stream resource
Signal -> lifecycle/control event
```

POSIX resource assumptions must not import ambient authority into Atarix.

## Initial Resource Sprint Scope

Resource Sprint 1 should implement only:

```text
Resource type identifiers
Resource object metadata
Resource ownership metadata
Basic allocation records
Basic release records
Basic lease expiration semantics
Basic accounting counters
Basic tests
```

Resource Sprint 1 should not implement distributed resource management, complex quotas, persistent reservations, cross-fabric federation, POSIX emulation, scheduler integration, storage allocation, or network bandwidth management.

## Required Tests

Initial tests should verify:

```text
Create resource record
Allocate available resource
Deny allocation of unavailable resource
Release resource
Released resource returns to available state
Allocation records owner
Allocation records authority used
Lease expiration reclaims resource
Revoked capability prevents further allocation
Resource lookup does not grant allocation authority
Resource visibility does not grant control authority
Quota denial fails closed
Cleanup removes temporary allocation
Destroyed owner triggers resource reclamation
Malformed resource request is rejected
```

## Open Questions

Q-001: What is the wire format for resource identity?

Q-002: Are resource identities object identities, resource-local identities, or both?

Q-003: What minimum resource metadata must be present in C structures?

Q-004: What are the initial resource classes required for Rev A?

Q-005: Does CPU-local SRAM appear as one resource pool or multiple regions?

Q-006: How are DMA windows represented?

Q-007: How should mailbox capacity be accounted?

Q-008: What is the minimum quota model for early implementation?

Q-009: How should unreclaimable resources be represented?

Q-010: Which resources are reserved for supervisor recovery paths?

Q-011: How does audit survive resource exhaustion?

Q-012: Which resource operations require explicit capability bits?

## Summary

The Resource Model makes resource use explicit, owned, auditable, bounded, and recoverable.

Its central rule is:

```text
Resource use is authority-bearing and must be explicit.
```

Its companion rule is:

```text
Observation is not control.
```
