# ATX-SPEC-015 POSIX Compatibility Model

## Status

Draft v0.1 scaffold

## Purpose

This document defines the Atarix POSIX Compatibility Model.

The POSIX Compatibility Model describes how Unix-style applications and interfaces are represented on top of native Atarix objects, services, capabilities, policies, mailboxes, audit, lifecycle, and recovery semantics.

## Core Rule

```text
POSIX is a compatibility personality.
POSIX is not native authority.
```

POSIX compatibility exists to run software.

POSIX compatibility must not weaken native Atarix authority, policy, audit, lifecycle, recovery, or supervisor isolation rules.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-005 Mailbox Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

This specification informs:

- ATX-SPEC-020 Hash Table and Lookup Acceleration Model
- ATX-SPEC-021 Memory and Data Movement Model

## Design Philosophy

POSIX compatibility is a boundary layer.

The POSIX view is a projection.

The Atarix model remains authoritative.

A compatibility environment may expose familiar concepts such as:

```text
Processes
Threads
Files
Directories
Descriptors
Pipes
Sockets
Signals
Environment variables
Working directories
Permission metadata
```

Internally, these must map to:

```text
Objects
Capabilities
Services
Mailbox endpoints
Resource allocations
Policy decisions
Audit records
Lifecycle states
Recovery state
```

The compatibility model should preserve useful Unix application behavior without importing Unix assumptions as native Atarix security semantics.

## Compatibility Environment

A POSIX compatibility environment is an object or service-owned environment.

Suggested service path:

```text
/service/compatibility/posix
```

A compatibility environment has:

```text
Environment identity
Version
Generation
Owner
Policy scope
Lifecycle state
Resource limits
Namespace view
Capability set
Audit policy
Recovery policy
```

The compatibility environment is the container for the POSIX projection.

It is not a privileged native execution mode.

It may be created, started, suspended, degraded, recovered, quarantined, or destroyed according to ordinary Atarix lifecycle and policy rules.

## Compatibility Boundary

The compatibility boundary is the point where POSIX-facing assumptions are translated into Atarix-native operations.

Across this boundary:

```text
POSIX names become namespace lookups.
POSIX descriptors become compatibility handles backed by capabilities.
POSIX process identifiers become compatibility labels for execution objects.
POSIX permission metadata becomes policy input.
POSIX streams become service or mailbox-backed objects.
POSIX sockets become network service endpoints.
```

The translation must be explicit.

The compatibility layer must not allow an application to obtain native authority merely because a POSIX interface normally implies it on another system.

## Native Model Remains Authoritative

Atarix-native state is authoritative when POSIX state and native state disagree.

Examples:

```text
If a descriptor table says a descriptor exists but the backing capability is revoked, the descriptor is unusable.
If a path exists but policy denies access, the operation is denied.
If a compatibility process exists but its native object is quarantined, the process cannot continue normal execution.
If a compatibility namespace view is stale, native directory state governs reconciliation.
```

Compatibility metadata may help explain intent.

It does not override native authority, policy, audit, lifecycle, or recovery state.

## Identity Labels

POSIX user and group concepts are compatibility identity labels.

They are not native authority.

A compatibility user or group label may map to:

```text
Atarix identity label
Policy subject
Compatibility account object
Audit actor label
Namespace view selector
Resource accounting group
```

A label may help policy decide whether an action should be allowed.

The label itself does not grant native authority.

Native capability and policy checks remain required for authority-bearing operations.

## Permission Metadata

POSIX permission bits are compatibility metadata and policy inputs.

They may describe intended access behavior inside the compatibility environment.

They do not replace native Atarix capabilities or policy.

Permission metadata may include:

```text
Read bit
Write bit
Execute bit
Owner category
Group category
Other category
Creation mask
Special compatibility flags
```

The compatibility layer may use this metadata to decide how to present behavior to POSIX-facing software.

The final native decision still depends on Atarix authority, policy, object state, lifecycle state, and recovery state.

## Paths

POSIX paths are compatibility namespace references.

A path may be absolute, relative, symbolic, generated, mounted, or view-specific.

Path resolution maps to Atarix namespace and directory operations.

Core rule:

```text
Path lookup is not access.
```

A successful path lookup may identify an object.

It does not grant permission to read, write, execute, enumerate, modify, delete, signal, mount, or control that object.

A path lookup result should preserve:

```text
Resolved object identity
Directory binding identity
Generation
Namespace view
Lookup policy result
Audit context where required
```

If namespace state and compatibility state disagree, native directory and object state govern reconciliation.

## Descriptors

A POSIX descriptor is a compatibility handle.

Internally it maps to Atarix-native capabilities, service bindings, object references, and resource allocations.

A descriptor may represent:

```text
Storage object access
Directory enumeration state
Stream endpoint
Pipe endpoint
Socket endpoint
Event object
Device-like service endpoint
Compatibility runtime object
```

A descriptor record should include:

```text
Descriptor number
Descriptor generation
Backing object identity
Backing service identity
Capability reference
Open mode
Inheritance policy
Lifecycle state
Audit policy
Resource accounting reference
```

Descriptor possession alone is not native authority.

A descriptor is usable only while its backing native authority remains valid and policy permits the operation.

## Descriptor Duplication And Inheritance

Descriptor duplication, passing, and inheritance are authority-sensitive compatibility operations.

They must be policy-controlled.

The compatibility layer must distinguish:

```text
Duplicate within same environment
Inherit into created execution object
Pass through IPC-like mechanism
Close or revoke descriptor
Mark descriptor non-inheritable
```

A duplicated descriptor must not silently widen authority.

A descriptor inherited by a new compatibility execution object must carry only the authority allowed by policy.

If backing authority is revoked, stale descriptors must become unusable or enter an explicit error state.

## Processes

A POSIX process is a compatibility execution object.

It has both a POSIX-facing identity and a native Atarix object identity.

A process record should include:

```text
Compatibility process identifier
Native object identity
Parent relationship where applicable
Compatibility environment identity
Identity labels
Capability set
Descriptor table reference
Namespace view
Resource allocation
Lifecycle state
Audit context
Recovery state
```

A process identifier is not authority.

Process existence does not imply a right to observe, interrupt, inspect, debug, or control the process.

Those operations remain native authority-bearing operations mediated by capability and policy.

## Threads

POSIX threads are compatibility execution contexts within a process object.

A thread record should include:

```text
Compatibility thread identifier
Native execution context identity
Owning process identity
Resource accounting reference
Scheduling policy reference
Lifecycle state
Audit context where required
```

Thread identity is not authority.

Thread creation, termination, suspension, and scheduling changes are resource- and policy-controlled operations.

## Process Creation

Traditional POSIX process creation must be represented carefully in a capability system.

Atarix may support multiple implementation strategies:

```text
Full compatibility clone
Copy-on-write compatibility clone
Restricted clone
Spawn-based emulation
Denied operation
```

The selected strategy must be declared by the POSIX compatibility profile.

Process creation must not silently duplicate authority beyond policy.

Inherited descriptors, namespace views, identity labels, resource limits, and capabilities must be explicitly copied, filtered, or denied according to policy.

## Program Loading And Replacement

Program loading maps a compatibility executable request to native Atarix object, storage, loader, and execution services.

Program replacement changes the executable image of an existing compatibility process.

Program loading or replacement must validate:

```text
Executable object identity
Executable version or format
Loader or interpreter identity
Execute authority
Policy approval
Descriptor inheritance policy
Environment metadata policy
Resource limits
Audit requirements
```

Program replacement must not preserve authority that policy requires to be dropped.

If loading cannot be verified, the operation must fail explicitly or enter a compatibility error state.

## Process Lifecycle

Suggested process lifecycle states:

```text
CREATED
LOADING
ACTIVE
WAITING
SUSPENDED
EXITING
TERMINATED
DEGRADED
QUARANTINED
FAILED
UNKNOWN
```

Compatibility lifecycle state must reconcile with the native object lifecycle state.

Unknown lifecycle state must be explicit.

## Mailbox-Based IPC Mapping

Atarix uses mailboxes as the native IPC abstraction.

Compatibility communication mechanisms are projections over mailboxes, services, objects, or future memory and data movement facilities.

Core rule:

```text
Native IPC is mailbox-based.
Compatibility IPC is a projection.
```

This keeps compatibility communication aligned with one native authority, audit, lifecycle, recovery, and resource-accounting model.

## Compatibility Events

POSIX-facing event delivery should map to mailbox-delivered events associated with a compatibility process, process group, or environment.

An event delivery record should include:

```text
Event identity
Sending compatibility identity
Target compatibility identity
Native actor identity
Target native object identity
Delivery policy result
Lifecycle state
Audit policy
```

Event delivery is not native control authority.

Event delivery requires native authority and policy approval where it affects another object.

## Compatibility Streams

POSIX-facing streams should map to mailbox-backed stream objects.

A stream endpoint should include:

```text
Read endpoint descriptor
Write endpoint descriptor
Backing mailbox identity
Buffer ownership policy
Flow-control policy
Resource accounting
Lifecycle state
Audit policy where required
```

Stream endpoints require capabilities.

Closing one endpoint must update lifecycle state and notify affected consumers according to mailbox semantics.

## Compatibility Queues

POSIX-style queues and notifications should map to mailbox queues or mailbox-delivered event objects.

Compatibility queues must preserve native rules for:

```text
Capability validation
Policy checks
Resource accounting
Backpressure
Audit
Recovery
```

A compatibility queue is not a separate native IPC subsystem unless later architecture explicitly creates one.

## Network Endpoint Projection

POSIX-facing network descriptors are compatibility network endpoints.

They map to Atarix Network Service bindings and mailbox-backed descriptors.

A network descriptor should include:

```text
Network family projection
Network type projection
Protocol projection
Network service identity
Endpoint capability
Mailbox or buffer identity
Resource accounting
Lifecycle state
Audit policy
```

A network descriptor is not direct network authority.

Network operations require service authority, endpoint capability, policy approval, and resource availability.

## Local Endpoint Projection

POSIX local communication endpoints should map to mailbox-backed local service endpoints.

Path-like local endpoint names are compatibility namespace entries.

Path lookup remains not access.

## Service-Bound Compatibility Endpoints

Compatibility endpoint names that look like device entries are bindings to native services.

Examples:

```text
Null-like endpoint -> Null Service
Entropy-like endpoint -> Entropy Service
Terminal-like endpoint -> Terminal Service
Storage-like endpoint -> Storage Service
Network-like endpoint -> Network Service
```

These bindings must be service-mediated, capability-controlled, policy-controlled, auditable where required, and recoverable.

Observation is not control.

## Shared Memory Projection

POSIX shared memory and memory mapping require special handling.

The native memory and data movement rules belong in ATX-SPEC-021.

This specification treats shared memory as a compatibility projection that must later map to Atarix memory ownership, lifecycle, audit, recovery, and data movement rules.

Shared memory must not become a way to bypass capability checks, policy checks, or audit boundaries.

## Mapping Rules

The following rules apply:

```text
Path lookup is not access.
Descriptor possession is not native authority.
Process identity is not native authority.
Compatibility roles are not native omnipotence.
Signals are compatibility events, not native control.
Device-like endpoints must be service-mediated.
Recovery does not restore compatibility authority without reconciliation.
Native IPC is mailbox-based.
```

## Profiles

Atarix may define compatibility profiles.

Examples:

```text
POSIX_MINIMAL
POSIX_USERLAND
POSIX_NETWORKED
POSIX_DEVELOPMENT
POSIX_LEGACY
```

Each profile defines which compatibility features are supported, restricted, emulated, or denied.

## Audit Requirements

Authority-bearing compatibility operations must be auditable.

Audit should record both:

```text
Compatibility view
Native Atarix authority decision
```

## Recovery Requirements

Recovery must reconcile compatibility state with native Atarix state.

Examples:

```text
Descriptor table vs native capabilities
Path bindings vs directory state
Process table vs object lifecycle
Socket state vs network service state
Storage view vs storage objects
Audit continuity
```

If reconciliation fails, the compatibility environment remains degraded or quarantined.

## Initial POSIX Sprint Scope

POSIX Sprint 1 should define:

```text
Compatibility environment metadata
Compatibility profile enum
Identity label mapping
Path lookup mapping
Descriptor table model
Basic open/read/write/close mapping
Basic program replacement mapping
Basic error projection
Audit hooks
Basic tests
```

## Required Future Work

- Define POSIX profile registry.
- Define descriptor record format.
- Define compatibility error mapping table.
- Define compatibility audit event additions.
- Define descriptor inheritance policy.
- Define process creation compatibility strategy.
- Define socket compatibility strategy.
- Define minimal libc target.
- Define FOSS application porting guide.
- Define compatibility IPC mappings against ATX-SPEC-005.
- Define shared memory mapping against ATX-SPEC-021.

## Summary

POSIX compatibility is a boundary layer for running existing software.

It is not native Atarix authority.

Its central rules are:

```text
POSIX is a compatibility personality.
Path lookup is not access.
Descriptors are compatibility handles.
Native IPC is mailbox-based.
POSIX compatibility must not weaken native Atarix security semantics.
```
