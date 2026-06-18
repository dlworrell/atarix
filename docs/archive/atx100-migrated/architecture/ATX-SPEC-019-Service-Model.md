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
