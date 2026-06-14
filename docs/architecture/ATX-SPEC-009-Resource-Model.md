# ATX-SPEC-009 Resource Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Resource Model.

The Resource Model describes how Atarix represents, allocates, accounts for, leases, revokes, audits, and cleans up finite system resources.

In Atarix, a resource is anything finite that an object may consume, reserve, control, expose, delegate, or depend upon.

Resource ownership is not authority by itself.

Resource visibility is not authority.

Resource allocation is not permanent authority.

Resource cleanup is a security requirement.

## Design Philosophy

Atarix treats resources as first-class architectural entities rather than incidental implementation details.

A resource must be:

- Named or discoverable where appropriate.
- Owned.
- Accounted.
- Constrained.
- Auditable.
- Revocable where possible.
- Cleaned up when no longer valid.

The Resource Model exists to answer:

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

This specification informs:

- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- Future scheduler, storage, network, filesystem, process, and POSIX compatibility specifications

## Terminology

### Resource

A finite system entity that can be allocated, consumed, reserved, delegated, or exhausted.

Examples include:

```text
CPU time
CPU cores
Memory regions
DMA channels
Interrupt lines
Mailbox endpoints
Network bandwidth
Storage blocks
File objects
Device queues
Framebuffer regions
Power budget
Thermal budget
Fabric links
Fabric credits
Timers
Locks
```

### Resource Object

An object that represents a resource or resource pool.

### Resource Pool

A collection of resources governed by common accounting, ownership, quota, or policy.

### Allocation

A granted use of a resource.

### Reservation

A claim on future or ongoing resource availability.

### Lease

A time-bounded allocation or reservation that expires unless renewed.

### Quota

A policy-imposed limit on resource consumption.

### Accounting

Measurement and attribution of resource consumption.

### Revocation

Termination or reduction of resource use.

### Reclamation

Recovery of a resource after release, revocation, expiration, crash, or owner disappearance.

## Resource Classes

Atarix recognizes the following conceptual resource classes.

### Compute Resources

Examples:

```text
CPU core
CPU thread
CPU time slice
scheduler queue slot
accelerator context
GPU compute queue
FPGA region
```

Compute resources may be time-shared, reserved, delegated, or partitioned.

### Memory Resources

Examples:

```text
CPU-local SRAM
fabric memory
DMA buffer
shared memory region
page frame
kernel heap allocation
object heap allocation
```

Memory resources must track ownership, lifetime, access authority, and cleanup state.

### Communication Resources

Examples:

```text
mailbox endpoint
message queue
fabric route
network socket
service channel
interrupt delivery path
```

Communication resources must not become ambient authority.

A mailbox endpoint may allow communication, but operations still require capability and policy validation.

### Storage Resources

Examples:

```text
block device region
file object
directory object
journal allocation
persistent object record
boot image slot
firmware image slot
```

Storage persistence must be explicit and auditable.

### Device Resources

Examples:

```text
serial port
network adapter
storage controller
sensor
GPIO line
display output
keyboard input
USB endpoint
```

Device resources are represented through device objects and mediated through capabilities and policy.

### Fabric Resources

Examples:

```text
fabric link
fabric credit
fabric route
fabric DMA window
fabric mailbox window
fabric interrupt route
fabric address aperture
```

Fabric resources must not leak physical topology into public authority.

### Supervisor Resources

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
```

Supervisor resources belong to the Supervisor Management Fabric, not the Operational Fabric.

Operational objects may observe supervisor resource state through an audit or observation bridge where policy permits, but they must not directly control supervisor resources.

Observation is not control.

### Environmental Resources

Examples:

```text
power budget
thermal budget
battery state
fan capacity
cooling margin
```

Environmental resources may constrain compute, storage, and fabric allocation decisions.

## Resource Identity And Naming

Resources may be named through the Directory Service.

Examples:

```text
/device/network/eth0
/fabric/node/0/cpu/0
/fabric/node/0/memory/local
/system/resource-manager
/system/power/budget
```

Resource names are not resource identities.

Resource identities are not authority.

Resource directory lookup does not grant resource access.

## Resource Ownership

Every resource allocation must be attributable to an owner.

The owner may be:

```text
System object
Service object
User-associated object
Session object
Application object
Supervisor authority
Fabric authority
```

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
```

Allocation may be:

```text
Exclusive
Shared
Read-only
Write-capable
Control-capable
Time-sliced
Quota-limited
Best-effort
Reserved
Borrowed
```

## Resource Reservations

Reservations are commitments of future or continued resource availability.

Reservations must be explicit.

A reservation must specify:

```text
Owner
Resource pool
Quantity
Duration
Renewal policy
Revocation behavior
Failure behavior
Audit policy
```

Reservations should not be indefinite unless explicitly persistent and policy-approved.

## Leases

Temporary resource allocations use leases by default.

A lease must define:

```text
Start time
Expiration time
Renewal authority
Expiration behavior
Cleanup authority
Audit behavior
```

Lease expiration must trigger reclamation, revocation, or quarantine.

Lease expiration must not silently convert into persistence.

## Quotas

Quotas constrain resource consumption.

Quotas may apply to:

```text
User-associated objects
Sessions
Services
Applications
Nodes
Fabrics
Resource pools
Policy domains
```

Quota enforcement must fail safely.

If quota state cannot be verified, new authority-bearing allocations should be denied.

## Accounting

Resource accounting must answer:

```text
Who allocated this?
Who is using this?
How much is consumed?
How long has it been held?
What authority allowed it?
What policy limits it?
What will clean it up?
```

Accounting data must be visible to authorized observers.

Accounting data should feed the Audit Model.

## Revocation

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

The Resource Model must explicitly record whether a resource supports revocation and what happens when revocation is requested.

Examples:

```text
CPU time can usually be revoked immediately.
Memory mappings may require invalidation and cleanup.
DMA windows must be closed carefully.
Storage writes may need journal reconciliation.
Firmware update slots may require supervisor-mediated recovery.
```

Revocation must be auditable.

## Reclamation

Resources must be reclaimed when:

```text
The owner exits.
The owner is destroyed.
The lease expires.
The capability is revoked.
The object is uninstalled.
The session ends.
The node crashes.
The fabric partitions.
Policy changes.
Quota enforcement requires it.
```

Reclamation may destroy, return, quarantine, scrub, or transfer the resource by explicit policy.

## Resource Cleanup

Resource cleanup is a security function.

Cleanup must address:

```text
Outstanding capabilities
Outstanding mailboxes
Pending DMA
Shared memory mappings
Directory bindings
Persistent state
Audit records
Temporary buffers
Locks
Reservations
```

Cleanup failure must be observable.

Cleanup failure must not silently preserve authority.

## Resource Exhaustion

Resource exhaustion is expected and must be safe.

When resources are exhausted, Atarix must:

- Fail closed for authority-bearing operations.
- Preserve system-critical recovery paths where possible.
- Avoid granting broader authority as a fallback.
- Audit denied or degraded allocations.
- Prefer controlled degradation over undefined behavior.

Examples:

```text
Out of memory must not grant access to shared memory.
Mailbox exhaustion must not bypass message validation.
Directory cache exhaustion must not bypass lookup policy.
Audit buffer exhaustion must not disable all auditing silently.
```

## Resource Visibility

Authorized observers may inspect resource state.

Observation may include:

```text
Resource identity
Resource type
Owner
Allocation state
Quota use
Lease expiration
Lifecycle state
Audit state
Health state
```

Observation does not imply control.

The ability to observe a resource does not imply the ability to allocate, revoke, mutate, or destroy it.

## Resource And Directory Interaction

Resources may be published through the Directory Service.

Directory lookup returns resource metadata or object identity.

Directory lookup does not grant allocation authority.

The access path is:

```text
Directory lookup
  -> Resource object identity
  -> Ring policy
  -> Ownership / authority policy
  -> Capability validation
  -> Resource policy
  -> Lifecycle validation
  -> Allocation decision
```

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

A resource capability must be constrained by:

```text
Resource identity or pool
Operation
Scope
Rights mask
Ring policy
Lifecycle state
Expiration
Delegation rules
Revocation rules
```

## Resource And Policy Interaction

Policy determines conditions under which resource authority may be exercised.

Examples:

```text
Maximum memory per session
Maximum mailbox depth per service
Maximum DMA window size
Allowed network bandwidth
Reserved CPU time for supervisor recovery
Read-only boot storage before verification
No persistent storage for temporary objects
```

Policy cannot grant authority where the Security and Capability models deny it unless explicitly defined by higher authority.

## Resource And Lifecycle Interaction

Resource lifecycle must track allocation state.

Suggested states:

```text
DISCOVERED
AVAILABLE
RESERVED
ALLOCATED
ACTIVE
SUSPENDED
REVOKING
REVOKED
EXPIRED
QUARANTINED
RECLAIMING
RECLAIMED
FAILED
```

Not every resource supports every state.

State transitions must be explicit and auditable.

## Persistence Rules

Persistent resource allocations require explicit policy.

Examples:

```text
Persistent storage reservation
Persistent service mailbox
Persistent boot image slot
Persistent device assignment
Persistent audit log allocation
```

Persistent allocations must specify:

```text
Owner
Reason
Scope
Authority
Recovery behavior
Removal path
Audit visibility
```

Persistence must not be accidental.

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

## Distributed Resource Considerations

Resources may exist on:

```text
Local node
Remote node in same fabric
Future federated fabric
Supervisor Management Fabric
```

Rev A implementations are only required to support local resources within one operational fabric.

Future distributed resource management must preserve:

```text
Ownership
Authority
Quotas
Auditability
Revocation
Cleanup
Failure isolation
```

## Supervisor Management Fabric Resources

Supervisor resources are control-isolated from the Operational Fabric.

Operational objects may request supervisor action only through explicit mediated interfaces.

Operational objects may observe supervisor state only through authorized observation or audit paths.

The following rule is normative:

```text
Observation is not control.
```

Examples of supervisor-controlled resources:

```text
Reset lines
Power rails
Watchdog timers
RTC state
Firmware validation state
Recovery mode state
Fault logs
```

Supervisor resource audit records may flow into the operational audit system through a one-way or tightly mediated Audit Bridge.

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

Resource Sprint 1 should implement only the smallest useful subset:

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

Resource Sprint 1 should not implement:

```text
Distributed resource management
Complex quota hierarchy
Persistent reservations
Cross-fabric resource federation
POSIX resource emulation
Scheduler integration
Storage allocation engine
Network bandwidth management
```

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

Atarix resources are not merely implementation allocations. They are architectural objects subject to authority, lifecycle, audit, cleanup, and policy.
