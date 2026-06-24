---
document: ATX-100-CH09
title: System Layers
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-001
  - ATX-SPEC-002
  - ATX-SPEC-003
  - ATX-SPEC-005
  - ATX-SPEC-015
---

# Chapter 9: System Layers

## Layer Model

Atarix is organized as layered architecture rather than a monolithic bus-oriented machine.

```text
Applications
  -> Compatibility Personalities
  -> Native Services
  -> Object Model
  -> Capability and Policy Enforcement
  -> Mailbox Transport
  -> Fabric Services
  -> Hardware Modules
```

## Architectural Intent

The layer model prevents implementation convenience from becoming architectural authority.

A higher layer may request service from a lower layer. It may not bypass native authority, lifecycle, recovery, or audit requirements.

## Compatibility Personalities

Compatibility personalities translate foreign programming models into native Atarix objects, capabilities, and service requests.

A compatibility personality may emulate POSIX, firmware-style APIs, or other environments. It does not inherit the authority assumptions of those environments.

## Native Services

Native services expose Atarix semantics directly.

They participate in capability enforcement, mailbox routing, audit windows, recovery domains, and version-aware behavior.

## Hardware Services

Hardware services are accessed through explicit fabric-mediated interfaces.

Hardware acceleration may reduce latency or CPU overhead, but accelerated paths must preserve the same authority, audit, lifecycle, and recovery semantics as non-accelerated paths.

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-013-Policy-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-013-Policy-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-013-Policy-Model.md`

# ATX-SPEC-013 Policy Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Policy Model.

The Policy Model defines how Atarix evaluates, approves, denies, constrains, delegates, revokes, audits, and reconciles authority-bearing operations.

Policy is not authority.

Policy governs whether authority may be exercised.

## Core Rule

```text
Authority is explicit.
Policy determines whether authority may be exercised.
```

A valid capability describes what could be done.

Policy decides whether it may be done now.

## Architectural Axioms

```text
Policy is not identity.
Policy is not authority.
Policy is not ownership.
Policy is not a capability.
Policy evaluation must be auditable.
Unknown policy state must be explicit.
Policy failure must fail closed.
```

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model

This specification informs:

- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## What Policy Governs

Policy may govern:

```text
Capabilities
Delegation
Revocation
Directory mutation
Resource allocation
Resource reservation
Service registration
Mailbox communication
Lifecycle transitions
Persistence
Recovery actions
Supervisor requests
Compatibility modes
Version negotiation
```

## Policy Layers

Atarix recognizes policy layers.

### Layer 0: Architectural Policy

Hard architectural invariants.

These cannot be overridden.

Examples:

```text
Identity is not authority.
Lookup is not access.
Observation is not control.
Unknown authority fails closed.
```

### Layer 1: Security Policy

Global security rules.

Examples:

```text
No delegation across rings without explicit approval.
Recovery operations require approved authority.
Supervisor requests require mediated approval.
```

### Layer 2: Administrative Policy

Site-specific administrative rules.

Examples:

```text
Maximum RAM allocation
Audit retention period
Allowed service providers
Persistent storage limits
```

### Layer 3: Object Policy

Object-specific rules.

Examples:

```text
Storage object is read-only.
Directory branch is immutable.
Resource pool has quota.
```

### Layer 4: Session Policy

Temporary or session-scoped rules.

Examples:

```text
Maintenance window
Recovery session
Emergency operation
Temporary override
```

Session policy should expire by default.

## Evaluation Order

Policy evaluation must be deterministic.

Conceptually:

```text
Architectural Policy
  -> Security Policy
  -> Administrative Policy
  -> Object Policy
  -> Session Policy
```

Deny overrides allow.

Unknown policy state fails closed for authority-bearing operations.

## Policy Results

Policy evaluation may return:

```text
ALLOW
DENY
DEFER
REQUIRES_APPROVAL
QUARANTINE
UNKNOWN
```

Policy results are not merely Boolean.

## Policy Object

Policy records are objects or object-owned records.

A policy record has:

```text
Policy ID
Version
Generation
Owner
Scope
Lifecycle state
Audit policy
```

Version describes schema compatibility.

Generation describes policy evolution history.

## Policy Scope

Policy may apply to:

```text
System
Fabric
Node
Service
Directory
Resource
Object
User-associated label
Session
Capability
Supervisor request
Compatibility environment
```

## Policy Inheritance

Policies may inherit or compose by scope.

Example:

```text
System
  -> Storage
      -> Dataset
          -> Object
```

Child policy may restrict further.

Child policy may not weaken architectural policy.

## Policy And Authority

Policy does not create authority by itself.

Policy can allow, deny, restrict, require approval, quarantine, or defer use of existing authority.

Policy cannot turn identity, lookup, observation, connectivity, or execution into authority.

## Policy And Capabilities

A capability may be valid but still denied by policy.

A policy may not allow an operation when the required authority does not exist unless a higher authority explicitly defines that exceptional path.

## Policy And Delegation

Delegation requires policy approval.

Policy may:

```text
Allow delegation
Restrict delegation
Deny delegation
Require approval
Require reduced authority
Require expiration
Require audit
```

Delegation may never exceed the source authority.

## Policy And Resources

Policy may constrain resources such as:

```text
Memory
CPU
Storage
Bandwidth
DMA windows
Mailbox capacity
Audit resources
Compatibility environments
Auxiliary compute providers
```

Resource exhaustion policy must not grant broader authority.

## Policy And Supervisor Requests

Supervisor actions are policy-governed.

Examples:

```text
Reset
Power off
Recovery mode
Firmware update
Clock modification
Watchdog modification
```

The Operational Fabric may request supervisor action only through mediated policy-controlled paths.

Supervisor control isolation remains in effect.

## Policy And Compatibility

Compatibility environments are policy-controlled.

Examples:

```text
POSIX personality
Virtual hardware
Legacy protocol translation
Compatibility VM
```

Policy may disable compatibility, restrict it, require quarantine, require audit, or require explicit approval.

Compatibility must not weaken native Atarix semantics.

## Policy And Versioning

Every policy record must contain:

```text
Policy Version
Policy Generation
```

Unknown policy versions must fail closed for authority-bearing operations.

Policy downgrade, migration, or rollback must be auditable.

## Policy And Audit

Every authority-bearing policy decision should be auditable.

Minimum audit fields:

```text
Policy ID
Policy Version
Policy Generation
Actor identity label
Authority used
Operation
Target object or resource
Result
Reason
Timestamp
```

## Policy And Errors

If policy cannot be evaluated, the result is UNKNOWN.

UNKNOWN fails closed for authority-bearing operations.

Policy errors must distinguish:

```text
Policy missing
Policy malformed
Policy unsupported
Policy version unsupported
Policy unavailable
Policy conflict
Policy evaluation failed
```

## Policy Cleanup Escalation Model

Policy may fail, deny, defer, or become unavailable while cleanup is required.

Cleanup is a security function.

Policy failure must not cause garbage, leaked authority, leaked resources, or unbounded persistence.

Atarix therefore defines a cleanup fallback ladder.

### Cleanup Levels

```text
L1 Normal Cleanup
L2 Owner Cleanup
L3 Parent / Session Cleanup
L4 System Lifecycle Cleanup
L5 Recovery Cleanup
L6 Supervisor Recovery Cleanup
L7 Evidence Preservation
L8 Last-Resort Containment
```

### L1 Normal Cleanup

Normal cleanup under the object's active cleanup policy.

Typical actions:

```text
Release temporary resources
Close mailboxes
Remove temporary directory bindings
Expire leases
Flush ordinary audit records
```

### L2 Owner Cleanup

Cleanup under owner authority or owner cleanup policy when object-local cleanup is denied, incomplete, or unavailable.

Typical actions:

```text
Retry cleanup with owner cleanup capability
Release owner-scoped temporary allocations
Mark unresolved state for audit
```

### L3 Parent / Session Cleanup

Cleanup under parent object, session, or scope authority when the owner is unavailable.

Typical actions:

```text
Expire session leases
Mark objects orphan-pending
Revoke session-external capabilities
Audit owner unavailability
```

### L4 System Lifecycle Cleanup

System lifecycle cleanup when object, owner, and session cleanup cannot safely complete.

Typical actions:

```text
Transition object to cleanup-pending
Revoke outward authority
Release safe resources
Mark unsafe resources for quarantine
```

### L5 Recovery Cleanup

Recovery cleanup when ordinary lifecycle cleanup cannot establish safe state.

Typical actions:

```text
Quarantine object or resource
Deny new authority
Preserve recovery metadata
Require reconciliation
```

### L6 Supervisor Recovery Cleanup

Supervisor-mediated recovery cleanup for failures affecting recovery paths, boot state, power state, watchdogs, or critical system state.

Typical actions:

```text
Local-only recovery action
Recovery-mode transition
Supervisor audit event
No new runtime authority
```

### L7 Evidence Preservation

Evidence-preservation fallback when destructive cleanup may destroy information needed for audit, recovery, or incident reconstruction.

Typical actions:

```text
Stop destructive cleanup
Snapshot or preserve state where possible
Quarantine preserved state
Audit heavily
Require manual or recovery review
```

### L8 Last-Resort Containment

Last-resort containment when cleanup safety is unknown and continued exposure is riskier than isolation.

Typical actions:

```text
Revoke external capabilities
Disconnect service endpoints
Freeze mutation
Preserve recovery path
Keep evidence intact where practical
```

## Cleanup Result Mapping

Policy result maps to cleanup behavior as follows:

```text
ALLOW
  -> Normal cleanup may proceed.

DENY
  -> Destructive cleanup denied. Attempt safe release only and escalate if needed.

DEFER
  -> Hold object, retry policy evaluation, and prevent new authority.

REQUIRES_APPROVAL
  -> Suspend object, preserve state, audit, and alert.

QUARANTINE
  -> Isolate object, revoke external authority, preserve evidence.

UNKNOWN
  -> Fail closed and use recovery-only cleanup path.
```

## Cleanup Escalation Rules

Cleanup escalation follows these rules:

```text
Cleanup must never create broader authority.
Cleanup must never silently delete evidence.
Cleanup must never silently preserve authority.
If cleanup safety is unknown, quarantine beats deletion.
All escalation steps must be auditable.
Escalation may reduce authority.
Escalation may revoke authority.
Escalation may quarantine.
Escalation may release temporary resources when safe.
Escalation may not grant new runtime authority.
```

## Policy Lifecycle

Policies participate in lifecycle management.

Suggested states:

```text
CREATED
ACTIVE
SUSPENDED
DEPRECATED
QUARANTINED
DESTROYED
UNKNOWN
```

Policies are not immortal.

Policy cleanup and replacement must be auditable.

## Policy Language

A future policy language may be defined later.

Rev A should prefer:

```text
Static policies
Structured policies
Table-driven policies
Explicit C-visible policy records
```

before introducing scripting, rules engines, or policy DSLs.

## Initial Policy Sprint Scope

Policy Sprint 1 should implement:

```text
Policy result enum
Policy record metadata
Version and generation fields
Basic allow / deny / unknown evaluation
Cleanup fallback level enum
Audit hooks for policy decisions
Basic tests
```

Policy Sprint 1 should not implement:

```text
Policy language
Distributed policy federation
Complex inheritance
Runtime scripting
Remote administrative policy
```

## Required Tests

Initial tests should verify:

```text
Allow decision
Deny decision
Unknown decision fails closed
Policy version unsupported fails closed
Capability valid but policy denied
Delegation denied by policy
Resource allocation denied by policy
Cleanup escalation from L1 to L5
Evidence-preservation fallback
Last-resort containment fallback
Audit record generated for policy decision
Audit record generated for cleanup escalation
```

## Required Future Work

- Define policy record wire format.
- Define policy result C API.
- Define policy conflict resolution.
- Define policy inheritance rules.
- Define supervisor request policy in ATX-SPEC-016.
- Define bootstrap policy in ATX-SPEC-014.
- Define persistent policy storage in ATX-SPEC-017.
- Define recovery policy reconciliation in ATX-SPEC-018.

## Summary

Capabilities describe what could be done.

Policy decides whether it may be done now.

When policy cannot decide, Atarix fails closed, audits the uncertainty, and escalates cleanup toward quarantine, evidence preservation, or supervisor-mediated recovery rather than permissive deletion or leaked authority.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-015-POSIX-Compatibility-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-015-POSIX-Compatibility-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-015-POSIX-Compatibility-Model.md`

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

All compatibility IPC objects shall conform to mailbox lifecycle rules defined by ATX-SPEC-005.

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

Compatibility endpoint names are service discovery artifacts. They are not service authority.

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

Compatibility audit records shall participate in the native audit chain.

The audit record should preserve native decision context even when the compatibility layer returns a simplified result to the application.

Recommended audit fields include:

```text
Compatibility environment identity
Compatibility process identity
Native object identity
Native service identity where applicable
Descriptor identity where applicable
Operation name
Requested compatibility mode
Native authority result
Policy result
Lifecycle state
Recovery state where relevant
Error mapping result where relevant
```

Compatibility audit should be especially careful around:

```text
Program loading
Descriptor inheritance
Path resolution
Network endpoint creation
Service-bound endpoint access
Policy denial
Recovery reconciliation
```

## Error Mapping

Compatibility errors are projections of native Atarix errors.

The native error remains the authoritative error.

A compatibility error mapping should preserve:

```text
Native error code
Compatibility error code
Object or service identity
Operation
Policy result if applicable
Lifecycle state if applicable
Audit event identity where applicable
```

Lossy error conversion must not erase native error context.

Examples:

```text
Policy denied -> compatibility access denial
Object not found -> compatibility not-found result
Resource exhausted -> compatibility resource failure
Unsupported version -> compatibility unsupported operation
Quarantined object -> compatibility unavailable or restricted result
Unknown state -> compatibility explicit failure
```

Compatibility error mapping should be table-driven and versioned.

Unknown native errors must not be silently converted to success.

Compatibility layers shall preserve native degraded-state semantics. A degraded native state must remain visible through compatibility error context rather than being flattened into an ordinary generic failure.

## Recovery And Reconciliation

Compatibility recovery must reconcile POSIX-facing state against native Atarix state before normal execution resumes.

Core rule:

```text
Compatibility recovery restores execution.
Compatibility recovery does not restore authority.
```

Compatibility recovery is a subordinate recovery domain of native Atarix recovery.

Recovery trusts native Atarix state, not reconstructed compatibility metadata.

Compatibility metadata may be used as evidence during recovery, but it must not grant or restore authority by itself.

## Recovery Phases

Suggested compatibility recovery phases:

```text
DETECT
FREEZE
INSPECT
RECONCILE
RESTORE
VERIFY
RESUME
```

A compatibility environment may not resume normal execution until required reconciliation succeeds or policy explicitly allows degraded operation.

## Descriptor Reconciliation

Descriptor reconciliation validates each compatibility descriptor against native backing state.

For each descriptor, recovery should verify:

```text
Descriptor record exists
Descriptor generation matches expected state
Backing capability exists
Backing capability is valid
Backing object or service exists
Backing object lifecycle permits use
Open mode remains policy-valid
Audit state is preserved where required
```

If reconciliation fails, the descriptor must become invalid, restricted, or quarantined.

Recovery must not silently recreate descriptor authority.

## Path Reconciliation

Path reconciliation validates compatibility namespace state against the native namespace, directory, and object model.

For each path binding, recovery should verify:

```text
Compatibility path record exists
Namespace view is still valid
Directory binding exists
Target object identity matches expected generation where required
Policy permits continued visibility
Target lifecycle permits use
```

If the path cannot be resolved safely, the path becomes unresolved or restricted.

Path recovery must not grant access.

## Process Reconciliation

Process reconciliation validates compatibility process state against native object lifecycle state.

For each process, recovery should verify:

```text
Compatibility process record exists
Native object identity exists
Native lifecycle state permits continuation
Descriptor table has reconciled
Namespace view has reconciled
Resource allocation is valid
Policy permits resumed execution
Audit context is continuous or explicitly degraded
```

A process must not resume merely because a compatibility process identifier exists.

## Network Reconciliation

Network endpoint reconciliation validates compatibility network descriptors against the Network Service.

Recovery should verify:

```text
Network descriptor exists
Endpoint capability remains valid
Network service identity is available
Connection or endpoint lifecycle is known
Resource accounting is valid
Policy permits continued use
```

Broken or ambiguous network endpoints should fail cleanly and require reconnect or explicit recovery policy.

## Audit Continuity

Compatibility recovery must verify audit continuity where required.

Recovery should check:

```text
Last known audit event
Expected next event relationship
Environment audit identity
Process audit identity
Recovery event record
Integrity status where applicable
```

If audit continuity cannot be established, policy decides whether the environment becomes degraded, quarantined, or stopped.

Audit uncertainty must be explicit.

## Quarantine Rules

A compatibility environment should enter quarantine when recovery discovers:

```text
Authority mismatch
Descriptor mismatch
Object identity mismatch
Namespace corruption
Audit discontinuity
Unknown recovery state
Unsupported compatibility version
Untrusted service binding
```

Quarantine blocks normal execution.

Policy may allow limited inspection, export, or repair operations.

Quarantine must not become a path to broader authority.

## Recovery Outcomes

Compatibility recovery may produce:

```text
RECOVERED
RECOVERED_DEGRADED
QUARANTINED
FAILED
MANUAL_REVIEW_REQUIRED
UNKNOWN
```

Unknown recovery outcome must fail closed for authority-bearing operations.

## Compatibility Profile Registry

Compatibility profiles define supported, restricted, emulated, and denied behavior for a POSIX environment.

A profile should specify:

```text
Profile name
Profile version
Supported API families
Descriptor behavior
Process creation strategy
Program loading strategy
IPC mappings
Network support
Service-bound endpoint support
Shared memory policy
Audit policy
Recovery policy
Unsupported operations
```

Profiles are versioned architecture objects.

Unknown profile versions must be explicit and must not silently fall back to broader compatibility.

## POSIX_MINIMAL

`POSIX_MINIMAL` is the smallest useful compatibility environment.

Expected support:

```text
Basic path lookup
Basic descriptors
Basic file-like read/write/close
Basic program loading
Basic environment metadata
Basic error projection
Basic audit hooks
```

Expected restrictions:

```text
No full process cloning requirement
No broad network requirement
No broad device endpoint requirement
No shared memory requirement
No legacy privilege expansion
```

This profile is suitable for early runtime tests, simple utilities, and compatibility scaffolding.

## POSIX_USERLAND

`POSIX_USERLAND` targets ordinary command-line and userland-style software.

Expected support:

```text
Directories
Regular file views
Descriptor inheritance policy
Process creation strategy
Program loading and replacement
Mailbox-backed streams
Compatibility events
Environment variables
Working directory state
```

Expected restrictions:

```text
Network optional
Service-bound endpoints limited
Shared memory profile-gated
Compatibility administrative role limited by native policy
```

## POSIX_NETWORKED

`POSIX_NETWORKED` extends userland compatibility with network endpoint projection.

Expected support:

```text
Network descriptors
Network service binding
Local endpoint projection
Mailbox-backed network buffers
Network audit events
Network recovery checks
```

Expected restrictions:

```text
No direct network authority from descriptor possession
Endpoint capability required
Policy required for bind/connect/listen-like operations
Recovery may require reconnect
```

## POSIX_DEVELOPMENT

`POSIX_DEVELOPMENT` supports build tools, compilers, linkers, shells, interpreters, package tools, and diagnostic utilities.

Expected support:

```text
Large descriptor tables
Process tree metadata
Program loading diagnostics
Filesystem-like workspace views
Expanded audit diagnostics
Development-oriented error detail
```

Expected restrictions:

```text
Debug-style observation requires explicit policy
Build tool authority remains scoped
Compatibility tooling cannot bypass native policy
```

## POSIX_LEGACY

`POSIX_LEGACY` is for software that expects older or broader Unix behavior.

This profile is higher risk and should be opt-in.

Expected support may include broader compatibility emulation.

Required controls:

```text
Explicit policy approval
Stronger audit
Tighter resource limits
Clear unsupported-operation list
Recovery quarantine preference
No native authority expansion
```

Legacy compatibility must not become a reason to weaken native Atarix semantics.

## Compatibility Test Matrix

ATX-SPEC-015 requires tests that prove the compatibility projection does not weaken native authority.

Initial test categories:

```text
Identity mapping
Permission metadata
Path lookup
Descriptor handling
Descriptor inheritance
Program loading
Process lifecycle
Mailbox IPC mapping
Network endpoint projection
Service-bound endpoint projection
Audit mapping
Error mapping
Recovery reconciliation
Quarantine behavior
Profile enforcement
Unsupported operation handling
```

## Required Identity Tests

Tests should verify:

```text
User label does not grant native authority
Group label does not grant native authority
Compatibility role does not bypass native policy
Audit records include compatibility and native identities
```

## Required Path Tests

Tests should verify:

```text
Path lookup identifies objects without granting access
Stale path binding reconciles against native directory state
Denied path access produces native policy result plus compatibility error
Namespace view cannot escape profile policy
```

## Required Descriptor Tests

Tests should verify:

```text
Descriptor maps to backing capability
Descriptor cannot outlive revoked capability
Descriptor duplication obeys policy
Descriptor inheritance obeys policy
Invalid descriptor produces explicit compatibility error
```

## Required Process And Program Tests

Tests should verify:

```text
Process identifier is not authority
Program loading requires execute authority
Program replacement drops denied authority
Process creation strategy matches profile
Quarantined process cannot resume normal execution
```

## Required IPC And Network Tests

Tests should verify:

```text
Compatibility events map to mailbox events
Streams map to mailbox-backed endpoints
Queues preserve mailbox resource limits
Network descriptors require endpoint capability
Local endpoint path lookup does not grant access
Service-bound endpoints are service-mediated
Shared memory remains governed by ATX-SPEC-021
```

## Required Audit And Error Tests

Tests should verify:

```text
Audit records preserve native decision context
Compatibility error mapping preserves native error context
Unknown native error never maps to success
Policy denial is auditable
Recovery actions are auditable
Degraded native state remains visible through compatibility error context
```

## Required Recovery Tests

Tests should verify:

```text
Recovery does not restore authority
Compatibility recovery remains subordinate to native recovery
Descriptor reconciliation fails closed
Path reconciliation does not grant access
Process reconciliation requires native lifecycle validity
Network reconciliation detects broken endpoints
Audit discontinuity becomes explicit
Quarantine blocks normal execution
Unknown recovery outcome fails closed
```

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

- Define POSIX profile registry object format.
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
- Define compatibility recovery test cases against ATX-SPEC-018.
- Define profile conformance test suite.

## Summary

POSIX compatibility is a boundary layer for running existing software.

It is not native Atarix authority.

Its central rules are:

```text
POSIX is a compatibility personality.
Path lookup is not access.
Descriptors are compatibility handles.
Native IPC is mailbox-based.
Compatibility recovery restores execution, not authority.
Compatibility profiles are explicit and versioned.
POSIX compatibility must not weaken native Atarix security semantics.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-019-Service-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-019-Service-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-019-Service-Model.md`

# ATX-SPEC-019 Service Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Service Model.

The Service Model describes how Atarix represents, discovers, authorizes, versions, audits, migrates, replaces, and retires services.

A service is a capability-mediated provider of functionality.

A service is not defined by where it runs.

## Core Rule

```text
Service location is not service identity.
```

A service may be provided by a CPU card, auxiliary compute card, storage card, network card, FPGA fabric service, supervisor-facing bridge, compatibility environment, or future provider.

Consumers should depend on the service identity, interface, version, policy, and capabilities, not the physical provider.

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
- ATX-SPEC-016 Supervisor Management Fabric

This specification informs:

- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model

## Design Philosophy

Atarix is a service-oriented fabric.

The primary CPU card is not required to implement every system capability locally.

Services may be offloaded to dedicated providers.

Examples include:

```text
Directory service
Audit service
Storage service
Network service
Hash service
Compression service
Time service
Netboot service
Recovery service
Compatibility service
```

This allows a W65C816 CPU card to participate in a system that provides larger memory, stronger storage, cryptographic hashing, compression, network boot, audit persistence, and future acceleration through explicit service interfaces.

## Service As Object

A service is an object or object-owned provider interface.

A service has:

```text
Service identity
Service name or names
Provider identity
Interface version
Lifecycle state
Authority requirements
Resource requirements
Policy scope
Audit policy
Error behavior
```

Service identity is independent of provider location.

## Service Namespaces

Recommended service names live under:

```text
/service
```

Examples:

```text
/service/directory
/service/audit
/service/storage
/service/network
/service/hash
/service/compression
/service/time
/service/netboot
/service/recovery
/service/compatibility
```

Directory lookup of a service does not grant authority to use it.

Lookup is not access.

## Service Providers

A service provider may be:

```text
CPU card
Auxiliary compute card
Raspberry Pi-class compute/buffer card
RISC-V service card
GPU or accelerator card
Storage processor card
Network card
FPGA fabric service
Supervisor audit bridge
Compatibility VM
POSIX personality
```

The provider is an implementation location, not the service identity.

## Service Registration

Service registration publishes the existence of a service.

A registration should include:

```text
Service identity
Service name
Provider identity
Interface version
Supported operations
Required capabilities
Resource class requirements
Policy reference
Lifecycle state
Health state
Generation
Audit policy
```

Registration requires authority.

A compromised provider must not be able to register arbitrary trusted services without policy approval.

## Service Discovery

Service discovery returns service metadata, not authority.

A discovery result may include:

```text
Service identity
Service type
Provider identity
Interface version
Available operations
Health state
Policy reference
Binding generation
```

Discovery does not return operational authority.

## Service Capabilities

Using a service requires explicit capability and policy approval.

Service authority may include:

```text
DISCOVER
OBSERVE
CALL
CONFIGURE
ALLOCATE
DELEGATE
REVOKE
MIGRATE
SUSPEND
RETIRE
```

Possession of a service name or provider identity does not imply any of these authorities.

## Service Calls

Service calls are normally transported through mailboxes or fabric message mechanisms.

Mailbox connectivity does not imply service authority.

Every authority-bearing service call must validate:

```text
Caller identity label
Target service identity
Operation
Interface version
Capability
Policy
Lifecycle state
Resource availability
```

## Service Versioning

Every service interface is versioned.

Service versioning follows ATX-SPEC-012.

A service advertises:

```text
Service interface major version
Service interface minor version
Service interface patch version
Feature set
Compatibility policy
```

Unknown major versions fail closed for authority-bearing operations.

Compatibility fallback must be explicit and auditable.

## Service Generation

Service generation is not service version.

Generation describes the evolution of a specific service registration or instance.

Version describes interface compatibility.

## Service Lifecycle

Suggested service lifecycle states:

```text
REGISTERED
INITIALIZING
ACTIVE
DEGRADED
SUSPENDED
MIGRATING
REVOKING
RETIRED
QUARANTINED
FAILED
UNKNOWN
```

Service lifecycle transitions must be auditable.

## Service Health

Services should publish health state where policy permits.

Health state may include:

```text
Available
Degraded
Busy
Resource constrained
Recovering
Quarantined
Failed
Unknown
```

Observation of health does not grant control authority.

## Service Migration

A service may migrate from one provider to another.

Migration must preserve:

```text
Service identity
Authority constraints
Policy constraints
Audit continuity
Version compatibility
Resource accounting
Lifecycle visibility
```

Migration must be auditable.

Migration must not silently create broader authority.

## Service Replacement

A service provider may be replaced.

Replacement must distinguish:

```text
Same service identity, new provider
New service identity, compatible interface
Compatibility adapter
Recovery replacement
```

Clients requiring stability should bind to service identity and compatible version, not physical provider.

## Service Retirement

Services are not immortal.

Retirement requires:

```text
Policy approval
Lifecycle transition
Capability revocation or migration
Resource cleanup
Directory update
Audit record
Compatibility decision where applicable
```

Retired services must not silently continue operating through stale bindings.

## Service Quarantine

A service may be quarantined when:

```text
Provider is compromised
Version is unsupported
Policy cannot be evaluated
Audit continuity is broken
Resource ownership is uncertain
Behavior is unsafe
Recovery requires isolation
```

Quarantine preserves evidence and prevents unsafe authority use.

## Service And Resources

Services consume and may expose resources.

Resource use must be explicit, accounted, policy-controlled, and auditable.

Examples:

```text
Audit service consumes reserved audit buffers.
Storage service consumes persistent storage.
Hash service consumes auxiliary compute cycles.
Network service consumes packet buffers and bandwidth.
Compression service consumes memory and compute time.
```

## Auxiliary Compute Services

Auxiliary compute cards may provide services such as:

```text
Hashing
Compression
Encryption support
Audit staging
Netboot cache
Filesystem service
Storage scrub assistance
Snapshot export
Replication support
```

Auxiliary compute resources are not automatically trusted.

They require service registration, version advertisement, capability mediation, policy approval, audit visibility, and cleanup.

## Supervisor-Facing Services

Supervisor-facing service exports are restricted.

Operational services may observe supervisor events through authorized audit or observation services.

Operational services may request supervisor actions only through mediated supervisor request paths.

A supervisor-facing service must not become direct supervisor control authority.

Observation is not control.

## Compatibility Services

Compatibility services may expose legacy interfaces, POSIX personalities, protocol translators, or virtual hardware.

Compatibility services are objects.

They require explicit capabilities, policy control, resource accounting, audit, lifecycle, and cleanup.

Compatibility must not weaken native Atarix authority semantics.

## Service And Policy

Policy governs:

```text
Who may register a service
Who may discover a service
Who may call a service
Who may configure a service
Who may migrate a service
Who may retire a service
Which versions are allowed
Which providers are trusted
Which compatibility modes are allowed
```

Policy failure must fail closed for authority-bearing service operations.

## Service And Audit

Service events that should be auditable include:

```text
Service registered
Service discovered where policy requires
Service call allowed
Service call denied
Service version negotiated
Service degraded
Service migrated
Service replaced
Service retired
Service quarantined
Service provider failed
Compatibility service activated
```

## Service And Errors

The Error Model must support service errors such as:

```text
SERVICE_NOT_FOUND
SERVICE_UNAVAILABLE
SERVICE_VERSION_UNSUPPORTED
SERVICE_POLICY_DENIED
SERVICE_PROVIDER_FAILED
SERVICE_QUARANTINED
SERVICE_MIGRATION_FAILED
SERVICE_UNKNOWN_STATE
```

Unknown service state must be explicit.

## Bootstrap Services

Bootstrap services are not runtime services.

Netboot, early time acquisition, firmware validation, and recovery image generation belong to the Bootstrap Security Model until runtime handoff.

Bootstrap authority is not runtime authority.

## Initial Service Sprint Scope

Service Sprint 1 should define:

```text
Service identity concept
Service registration metadata
Service version advertisement
Service lifecycle states
Basic service discovery metadata
Capability checks for service calls
Basic tests
```

Service Sprint 1 should not implement:

```text
Service migration
Distributed service federation
Complex service failover
POSIX compatibility
Remote service registry federation
General service scripting
```

## Required Tests

Initial tests should verify:

```text
Service registration requires authority
Service discovery does not grant authority
Service call requires capability
Unsupported service version fails closed
Service lifecycle transition is audited
Service quarantine denies calls
Provider identity is not service identity
Compatibility service requires policy
Auxiliary compute service is not automatically trusted
```

## Required Future Work

- Define service identity wire format.
- Define service registration record format.
- Define service discovery C API.
- Define service call metadata.
- Define service migration semantics.
- Define service retirement semantics.
- Define compatibility service semantics in ATX-SPEC-015.
- Define storage-backed service persistence in ATX-SPEC-017.
- Define service recovery and reconciliation in ATX-SPEC-018.

## Summary

The Service Model defines Atarix as a capability-secured service-oriented fabric.

Its central rules are:

```text
Service location is not service identity.
Discovery is not authority.
A provider is not automatically trusted.
Compatibility belongs at boundaries.
Unknown service state must be explicit.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-021-Memory-and-Data-Movement-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-021-Memory-and-Data-Movement-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-021-Memory-and-Data-Movement-Model.md`

# ATX-SPEC-021 Memory and Data Movement Model

## Status

Draft v0.2

## Purpose

This document defines the Atarix Memory and Data Movement Model.

Memory movement is a system resource.

ATX-SPEC-021 defines how Atarix moves data between CPUs, services, mailboxes, accelerators, storage, and fabric-attached devices without weakening authority, policy, audit, recovery, human readability, or compatibility semantics.

This specification covers:

- Memory ownership
- Data movement authority
- Copy, move, map, and share semantics
- Fabric transfer rules
- Zero-copy constraints
- Mailbox payload movement
- Lookup-accelerator memory access
- POSIX compatibility memory behavior
- Audit-window reconstruction for data movement
- Recovery-safe memory transfer behavior

## Core Rule

Data movement is not authority.

A component may copy, move, map, share, cache, stage, or accelerate access to data only after the relevant Atarix authority checks have succeeded.

Reachability is not permission.

No bus master, fabric node, lookup accelerator, cache, descriptor, or compatibility layer may treat physical addressability as authority.

## Human-Readable Abstraction Rule

Atarix must preserve human-readable architecture even when data movement is optimized.

Every optimized data movement path must provide an explainable projection.

Examples:

```text
copy operation      -> source, destination, length, authority, generation
shared mapping      -> owner, borrower, lifetime, revocation rule
fabric transfer     -> service route, mailbox sequence, audit event
cache promotion     -> original object, generation, validation rule
```

No memory optimization may become opaque authority.

## Audit Window Rule

Privileged or cross-domain data movement must be reconstructable inside a bounded audit window.

An audit window should answer:

- Who requested the movement?
- What object, buffer, page, record, or mailbox was involved?
- Which authority and policy checks applied?
- What source and destination domains participated?
- What generation, version, or lifecycle window applied?
- Was the operation completed, denied, aborted, retried, reconciled, or quarantined?

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
- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model
- ATX-SPEC-020 Hash Table and Lookup Acceleration Model

This specification informs:

- Future fabric controller RTL
- Persistent storage pipelines
- POSIX compatibility memory behavior
- Lookup accelerator memory access
- Service-to-service payload routing
- Zero-copy data-plane design

## Design Goals

Atarix memory and data movement should provide:

- Explicit ownership
- Explicit authority checks
- Bounded transfer semantics
- Auditable movement
- Recovery-safe completion rules
- Low copy overhead where safe
- Clear separation between control-plane and data-plane behavior
- Compatibility with mailbox routing
- Compatibility with POSIX expectations
- Hardware-friendly bus and fabric behavior

Performance is valuable only when it preserves architecture semantics.

## Initial Movement Forms

Atarix initially defines these movement forms.

### Copy

A copy creates a distinct destination representation while preserving the source.

Copies must preserve provenance metadata when crossing authority, policy, audit, or recovery boundaries.

### Move

A move transfers ownership or active responsibility from one domain to another.

Moves must have explicit lifecycle and recovery semantics.

### Map

A map creates an addressable view into data owned by another object, service, or memory domain.

Mappings must be revocable unless explicitly declared permanent by native object state and policy.

### Share

A share allows multiple authorized parties to access the same data under defined rules.

Sharing requires explicit authority, policy, lifetime, and recovery behavior.

### Fabric Transfer

A fabric transfer moves data without ordinary CPU copy loops.

A fabric engine is never authority.

Fabric movement must operate through explicit descriptors validated by capability, policy, lifecycle, generation, and recovery rules.

### Zero-Copy Path

Zero-copy is an optimization, not a permission model.

A zero-copy path may avoid data copying only when ownership, lifetime, authority, policy, audit, and recovery requirements remain explicit.

## Ownership Rule

Every data buffer must have an owner.

Ownership may belong to an object, service, mailbox domain, storage domain, compatibility personality, or fabric endpoint.

Borrowed access must identify:

- Owner
- Borrower
- Access mode
- Lifetime
- Generation
- Revocation rule
- Recovery outcome

## Descriptor Rule

External memory movement must be described by explicit descriptors.

A movement descriptor is not authority.

A descriptor may describe:

- Source identity
- Destination identity
- Source address or object reference
- Destination address or object reference
- Length
- Movement form
- Required access mode
- Expected generation
- Completion rule
- Audit projection rule
- Recovery rule

A descriptor must be validated before execution.

## Canonical Movement Descriptor v1

The initial public descriptor is `atarix_memory_descriptor_v1_t`.

It is intended to be small enough for mailbox-mediated setup and explicit enough for audit, validation, and future fabric execution.

Required fields:

```text
magic              descriptor format marker
version            descriptor version
flags              movement form and validation flags
source_object      source object, mailbox, buffer, or service identifier
source_offset      source-local byte offset
destination_object destination object, mailbox, buffer, or service identifier
destination_offset destination-local byte offset
length             requested byte length
capability_id      explicit capability authorizing the movement
generation         expected source or transfer generation
audit_hint         human-readable projection selector
```

Descriptor validation MUST reject:

- Null descriptors
- Invalid magic
- Unsupported version
- Zero-length transfers
- Missing capability identifiers
- Missing source identifiers
- Missing destination identifiers
- Invalid or reserved movement forms
- Lengths exceeding implementation limits

## Initial Movement Flags

Initial movement forms are encoded as flags:

```text
ATARIX_MEMORY_MOVE_COPY
ATARIX_MEMORY_MOVE_MOVE
ATARIX_MEMORY_MOVE_MAP
ATARIX_MEMORY_MOVE_SHARE
ATARIX_MEMORY_MOVE_FABRIC
ATARIX_MEMORY_MOVE_ZERO_COPY
```

Exactly one movement form should be selected for the initial validation profile.

## Initial Validation Statuses

Initial validation statuses:

```text
ATARIX_MEMORY_STATUS_OK
ATARIX_MEMORY_STATUS_NULL_DESCRIPTOR
ATARIX_MEMORY_STATUS_BAD_MAGIC
ATARIX_MEMORY_STATUS_BAD_VERSION
ATARIX_MEMORY_STATUS_ZERO_LENGTH
ATARIX_MEMORY_STATUS_MISSING_CAPABILITY
ATARIX_MEMORY_STATUS_MISSING_SOURCE
ATARIX_MEMORY_STATUS_MISSING_DESTINATION
ATARIX_MEMORY_STATUS_BAD_FLAGS
ATARIX_MEMORY_STATUS_LENGTH_EXCEEDED
```

## Mailbox Interaction

Mailbox messages may carry small payloads directly or may reference external buffers, descriptors, or data windows.

External references must be:

- Capability-checked
- Policy-checked
- Generation-aware
- Bounded by length
- Auditable
- Recovery-safe
- Non-authoritative by themselves

Mailbox control flow remains separate from bulk data movement.

## Lookup Accelerator Interaction

ATX-SPEC-020 lookup accelerators may fetch, inspect, or compare memory only through ATX-SPEC-021-compliant movement rules.

Lookup accelerators must not infer authority from reachable memory.

Accelerated reads must preserve:

- Registry identity
- Table generation
- Probe bounds
- Capability result
- Policy result
- Audit projection
- Recovery status

## POSIX Compatibility Interaction

POSIX compatibility memory behavior is a personality mapping over native Atarix memory movement.

POSIX read, write, mmap, pipe, socket, and descriptor behavior must not bypass native ownership, authority, audit, or recovery rules.

## Recovery Rule

Every cross-domain data movement operation must have a defined recovery outcome.

Valid outcomes include:

- Completed
- Denied
- Aborted before visibility
- Reconciled
- Retried under a new generation
- Quarantined

Unrecoverable ambiguous movement is forbidden.

## Initial Implementation Plan

The initial implementation should add:

- `include/atarix/memory.h`
- `src/memory.c`
- `tests/memory/test_memory_descriptor.c`
- Automake integration for the memory descriptor test
- AEMS traceability metadata
- Engineering Gate coverage through `make check`

## Initial Test Plan

The initial unit test must validate:

- A valid copy descriptor succeeds.
- Null descriptor fails.
- Bad magic fails.
- Bad version fails.
- Zero length fails.
- Missing capability fails.
- Missing source fails.
- Missing destination fails.
- Unknown flags fail.
- Excessive length fails.

## Open Questions

- Which transfers require pre-commit audit rather than post-commit audit?
- How should zero-copy lifetimes be represented in the object model?
- How should POSIX mmap semantics map onto native Atarix movement forms?
- What minimum hardware signals are required for an ECP5 transfer descriptor gate?

## TODO

- Implement canonical movement descriptor layout.
- Add descriptor validation helper.
- Add tests and wire them into `make check`.
- Add ATX-100 Chapter 14 once this specification stabilizes.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/architecture-backlog.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/architecture-backlog.md -->
### Integrated source: `docs/architecture/architecture-backlog.md`

# Atarix Architecture Backlog

## Status

Living planning document

## Purpose

This document tracks planned architecture specifications and major architecture follow-up work discovered during specification review.

The backlog prevents future work from being hidden inside implementation tickets or ad hoc discussions.

## Current Foundation

The current architecture foundation includes:

```text
ATX-SPEC-001 Security Model
ATX-SPEC-002 Authority Model
ATX-SPEC-003 Capability Model
ATX-SPEC-004 Lifecycle Model
ATX-SPEC-005 Mailbox Model
ATX-SPEC-006 Object Model
ATX-SPEC-007 Namespace Model
ATX-SPEC-008 Directory Service Model
ATX-SPEC-009 Resource Model
ATX-SPEC-010 Audit Model
ATX-SPEC-011 Error Model
```

Architecture Review 001 accepted the current foundation with required follow-up.

## Planned Specifications

### ATX-SPEC-012 Versioning Model

Priority: High

Purpose:

Define versioning for wire formats, object schemas, capability records, directory bindings, resource records, audit records, error records, firmware, services, storage formats, and mixed-version operation.

Key rule:

```text
Version uncertainty must be explicit.
```

### ATX-SPEC-013 Policy Model

Priority: High

Purpose:

Define policy evaluation, policy objects, policy scope, policy inheritance, policy versioning, administrative policy, site policy, and fail-closed behavior when policy cannot be evaluated.

### ATX-SPEC-014 Bootstrap Security Model

Priority: High

Purpose:

Define root of trust, boot-time authority, secure boot, measured boot, pre-OS networking, time synchronization, recovery mode, bootstrap audit, and handoff into runtime security.

Key rule:

```text
Bootstrap authority is not runtime authority.
```

### ATX-SPEC-015 POSIX Compatibility Model

Priority: Deferred

Purpose:

Define how POSIX/FOSS applications may run on top of Atarix without importing ambient-authority assumptions into the native architecture.

Key rule:

```text
POSIX is a compatibility target, not an architectural foundation.
```

### ATX-SPEC-016 Supervisor Management Fabric

Priority: High

Purpose:

Define the isolated Supervisor Management Fabric, supervisor authority, supervisor resources, supervisor command request path, supervisor audit export, physical/logical isolation, and off-board security requirements.

Key rule:

```text
The Supervisor Fabric is control-isolated but audit-visible.
```

### ATX-SPEC-017 Storage and Persistence Model

Priority: Medium-High

Purpose:

Define persistent object storage, checksummed storage, journaling, copy-on-write storage targets, RAM-backed audit buffers, ZFS-inspired recovery architecture, snapshot semantics, and persistence cleanup.

Key rule:

```text
Persistence must be explicit and recoverable.
```

### ATX-SPEC-018 Recovery and Reconciliation Model

Priority: Medium-High

Purpose:

Define crash recovery phases, quarantine, reconciliation of resources, directory bindings, capabilities, audit continuity, storage state, and post-crash authority restoration rules.

Key rule:

```text
Recovery must not silently regrant authority.
```

### ATX-SPEC-019 Service Model

Priority: Medium

Purpose:

Define services as discoverable, versioned, capability-mediated, location-independent providers of system functionality.

Examples:

```text
/service/audit
/service/storage
/service/network
/service/hash
/service/compression
/service/directory
```

Key rule:

```text
Service location is not service identity.
```

## Cross-Cutting Follow-Up Work

Architecture Review 001 identified these required follow-up areas:

```text
Object identity wire format
Path syntax and limits
Authority Model expansion
Lifecycle state machine
Ownership transfer semantics
Directory binding generation representation
Resource identity format
Audit record wire format
Error code namespace
Quarantine transition semantics
Supervisor Audit Bridge semantics
Bootstrap network security
Storage recovery semantics
```

## Architectural Axioms

Current recurring Atarix axioms:

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Execution is not authority.
Ownership is not authority.
Allocation is not ownership.
Persistence is not ownership.
Connectivity is not trust.
Bootstrap authority is not runtime authority.
Unknown state must be explicit.
When authority cannot be verified, fail closed.
```

## Recommended Priority Order

```text
1. ATX-SPEC-012 Versioning Model
2. ATX-SPEC-013 Policy Model
3. ATX-SPEC-014 Bootstrap Security Model
4. ATX-SPEC-016 Supervisor Management Fabric
5. ATX-SPEC-017 Storage and Persistence Model
6. ATX-SPEC-018 Recovery and Reconciliation Model
7. ATX-SPEC-019 Service Model
8. ATX-SPEC-015 POSIX Compatibility Model
```

## Implementation Rule

No implementation sprint should silently define behavior that belongs in a planned architecture specification.

If implementation requires unresolved architecture, update this backlog and write the specification before hardening the behavior into public interfaces.

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/architecture-review-001.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/architecture-review-001.md -->
### Integrated source: `docs/reviews/architecture-review-001.md`

# Architecture Review 001

## Status

Draft review v0.1

## Scope

This review covers the current Atarix architecture foundation:

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

## Purpose

This review checks whether the current specifications agree on terminology, authority, identity, ownership, lifecycle, observability, cleanup, failure, and recovery.

The goal is to catch contradictions before implementation hardens them into public APIs.

## Architectural Axioms Under Review

The following axioms are currently treated as controlling architectural rules:

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Execution is not authority.
Ownership is not authority.
Allocation is not ownership.
Persistence is not ownership.
Connectivity is not trust.
Bootstrap authority is not runtime authority.
Unknown state must be explicit.
When authority cannot be verified, fail closed.
```

## Review Summary

Overall status:

```text
ACCEPTED WITH REQUIRED FOLLOW-UP
```

The current specification set is coherent enough to continue architectural development, but several cross-cutting topics require future specifications or deeper expansion.

## Consistency Findings

### Identity And Names

Status: PASS

The Object, Namespace, and Directory specifications consistently separate object identity from names and bindings.

Confirmed rules:

```text
Names are bindings.
Object identities are opaque.
Lookup does not grant authority.
Directory entries are not capabilities.
```

Required follow-up:

- Define concrete object identity wire format.
- Define path syntax and limits.
- Define whether kernel-visible paths support Unicode or restricted ASCII.

### Authority

Status: PASS WITH FOLLOW-UP

The Security, Authority, Capability, Directory, Resource, and Error specifications consistently reject ambient authority.

Confirmed rules:

```text
Execution does not imply authority.
Identity does not imply authority.
Connectivity does not imply trust.
Lookup does not imply access.
Observation does not imply control.
```

Required follow-up:

- Expand ATX-SPEC-002 Authority Model beyond stub status.
- Define bootstrap authority separately from runtime authority.
- Define supervisor authority separately from operational fabric authority.

### Ownership

Status: PASS WITH FOLLOW-UP

The current specifications consistently treat ownership as accountability rather than unrestricted control.

Confirmed rules:

```text
Ownership is not authority.
Binding ownership and object ownership may differ.
Resource ownership and resource authority may differ.
```

Required follow-up:

- Define ownership transfer semantics.
- Define shared ownership or administrative sponsorship.
- Define owner disappearance behavior more rigorously in Lifecycle Model.

### Lifecycle And Cleanup

Status: PASS WITH FOLLOW-UP

Lifecycle, Resource, Directory, Audit, and Error specifications consistently treat cleanup as a security requirement.

Confirmed rules:

```text
No object is immortal by default.
No resource may silently leak.
Stale bindings are observable.
Cleanup failures are auditable.
Unknown state is explicit.
```

Required follow-up:

- Expand ATX-SPEC-004 Lifecycle Model beyond stub status.
- Define object lifecycle state machine.
- Define lease expiration semantics.
- Define uninstall and owner-disappearance behavior.

### Directory And Lookup

Status: PASS

ATX-SPEC-008 correctly separates discoverability from authority.

Confirmed rules:

```text
Lookup is not access.
Enumeration is not access.
Aliases do not grant authority.
Rebinding requires authority.
```

Required follow-up:

- Define Directory Sprint 1 public C interfaces.
- Define binding ID and generation representation.
- Define directory failure error codes after Error Model matures.

### Resource Management

Status: PASS WITH FOLLOW-UP

ATX-SPEC-009 correctly treats resources as first-class architectural entities with ownership, accounting, leases, quota, cleanup, and audit.

Confirmed rules:

```text
Resource use is authority-bearing.
Resource visibility is not control.
Resource exhaustion must fail safely.
Supervisor resources are control-isolated.
```

Required follow-up:

- Define minimum Rev A resource classes.
- Define resource identity format.
- Define reserved resources for audit, recovery, and supervisor paths.
- Define RAM-card-backed audit buffers once fabric memory services mature.

### Audit

Status: PASS WITH FOLLOW-UP

ATX-SPEC-010 establishes audit as evidence rather than debugging output.

Confirmed rules:

```text
Audit is evidence.
Observation is not control.
Audit suppression must be observable.
Supervisor events are auditable.
Cleanup failures are security events.
```

Required follow-up:

- Define audit record wire format.
- Define audit buffer hierarchy.
- Define RAM-backed audit ingest buffers.
- Define asynchronous audit persistence.
- Define ZFS-inspired or copy-on-write persistent audit storage in future Storage Model.

### Error Handling

Status: PASS WITH FOLLOW-UP

ATX-SPEC-011 correctly treats failure and unknown state as first-class architectural concepts.

Confirmed rules:

```text
Unknown state must be explicit.
Fail closed for authority-bearing operations.
Prefer quarantine over assumption.
Degraded operation must be observable.
Recovery must be auditable.
```

Required follow-up:

- Define concrete error code namespace.
- Define structured error object format.
- Define quarantine state transitions.
- Define recovery workflow semantics.

### Supervisor And Operational Fabric

Status: NEEDS SPECIFICATION

The current documents identify Supervisor Management Fabric and Supervisor Audit Bridge concepts, but a dedicated specification is required.

Confirmed direction:

```text
Operational Fabric contains discovery, control, service, and data planes.
Supervisor Management Fabric is control-isolated.
Operational Fabric may observe supervisor state through audit paths.
Operational Fabric may not directly control supervisor authority.
Observation is not control.
```

Required follow-up:

- Create ATX-SPEC-016 Supervisor Management Fabric.
- Define Supervisor Audit Bridge semantics.
- Define supervisor command request path.
- Define physical vs logical isolation requirements.
- Define cryptographic requirements for off-board supervisor access.

### Bootstrap Security

Status: NEEDS SPECIFICATION

The current Security, Audit, Error, and Resource specifications all reference bootstrap behavior, but bootstrap authority remains undefined.

Confirmed direction:

```text
Bootstrap authority is not runtime authority.
Pre-OS networking is not runtime networking.
Boot audit imports into runtime audit where practical.
Failure before Supervisor enters recovery-only mode.
```

Required follow-up:

- Create ATX-SPEC-014 Bootstrap Security Model.
- Define root of trust.
- Define secure boot and measured boot requirements.
- Define pre-OS network services for netboot and time sync.
- Define recovery mode restrictions.

### Storage And Persistence

Status: NEEDS SPECIFICATION

The current specifications repeatedly reference persistence, crash recovery, journal replay, RAM-backed buffers, and copy-on-write storage, but no Storage and Persistence Model exists yet.

Confirmed direction:

```text
Persistence must be explicit.
Persistent state must be recoverable.
Unknown persistent state must be quarantined.
Audit persistence should be checksummed and crash-recoverable.
ZFS-style semantics are a long-term target, not a Rev A dependency.
```

Required follow-up:

- Create ATX-SPEC-017 Storage and Persistence Model.
- Define persistent object storage.
- Define audit persistence hierarchy.
- Define snapshot and rollback semantics.
- Define recovery after partial writes.

### Recovery And Reconciliation

Status: NEEDS SPECIFICATION

Recovery and reconciliation appear in Lifecycle, Resource, Audit, and Error specifications, but remain cross-cutting.

Confirmed direction:

```text
Recovery must not silently regrant authority.
Unknown state remains visible.
Unreconciled resources are quarantined.
Recovery is auditable.
```

Required follow-up:

- Create ATX-SPEC-018 Recovery and Reconciliation Model.
- Define crash recovery phases.
- Define resource reconciliation.
- Define directory reconciliation.
- Define capability and authority reconciliation.
- Define audit continuity verification.

## Required Future Specifications

The review identifies the following future specifications as required:

```text
ATX-SPEC-012 Versioning Model
ATX-SPEC-013 Policy Model
ATX-SPEC-014 Bootstrap Security Model
ATX-SPEC-015 POSIX Compatibility Model
ATX-SPEC-016 Supervisor Management Fabric
ATX-SPEC-017 Storage and Persistence Model
ATX-SPEC-018 Recovery and Reconciliation Model
```

## Recommended Priority Order

Recommended next specification order:

```text
1. ATX-SPEC-012 Versioning Model
2. ATX-SPEC-013 Policy Model
3. ATX-SPEC-014 Bootstrap Security Model
4. ATX-SPEC-016 Supervisor Management Fabric
5. ATX-SPEC-017 Storage and Persistence Model
6. ATX-SPEC-018 Recovery and Reconciliation Model
7. ATX-SPEC-015 POSIX Compatibility Model
```

Rationale:

- Versioning affects every wire format and persistent record.
- Policy determines conditional authority.
- Bootstrap Security and Supervisor Fabric govern early trust and recovery.
- Storage and Recovery define durable state.
- POSIX compatibility should remain deferred until native semantics are stable.

## Implementation Guidance

No major subsystem implementation should proceed without checking:

```text
Does this expose identity as authority?
Does this turn lookup into access?
Does this turn observation into control?
Does this create ambient authority?
Does this create persistence without policy?
Does this create resources without cleanup?
Does this hide unknown state?
Does this fail open when authority is uncertain?
```

## Review Decision

```text
Decision: ACCEPTED WITH REQUIRED FOLLOW-UP
Reviewer: Architecture Review 001
Date: 2026-06-14
```

## Closing Note

The current architecture is coherent enough to continue specification work, but the next phase must focus on cross-cutting models: versioning, policy, bootstrap trust, supervisor isolation, storage persistence, and recovery.

The strongest recurring Atarix principle is separation of concepts that legacy systems often conflate:

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Unknown is not valid.
```
