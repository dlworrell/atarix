# ATX-SPEC-015 Reconciliation Review

## Status

Review complete

## Scope

This review reconciles ATX-SPEC-015 POSIX Compatibility Model against the architecture areas it most directly touches:

```text
ATX-SPEC-005 Mailbox Model
ATX-SPEC-010 Audit Model
ATX-SPEC-011 Error Model
ATX-SPEC-018 Recovery and Reconciliation Model
ATX-SPEC-019 Service Model
ATX-SPEC-021 Memory and Data Movement Model
```

The goal is to verify that POSIX compatibility remains a compatibility projection and does not become native Atarix authority.

## Review Result

```text
Overall Result: ACCEPTED WITH FOLLOW-UP REGISTRIES
```

ATX-SPEC-015 is coherent with the current Atarix architecture baseline.

No blocking conflict was identified.

Several follow-up registries should be created before implementation begins.

## Core Compatibility Rule Confirmed

ATX-SPEC-015 correctly establishes:

```text
POSIX is a compatibility personality.
POSIX is not native authority.
```

This aligns with the architecture-wide principles:

```text
Discovery is not authority.
Lookup is not access.
Observation is not control.
Persistence is not authority.
Recovery is not administrative access.
Compatibility is not authority.
```

## Reconciliation Against ATX-SPEC-005 Mailbox Model

ATX-SPEC-015 now states:

```text
Native IPC is mailbox-based.
Compatibility IPC is a projection.
```

This is consistent with the Atarix mailbox-centered IPC architecture.

POSIX-facing events, streams, queues, and local endpoints are modeled as projections over mailboxes or mailbox-backed service endpoints.

### Result

```text
Aligned
```

### Follow-Up

Define a compatibility IPC mapping table:

```text
Compatibility event -> mailbox event
Compatibility stream -> mailbox-backed stream endpoint
Compatibility queue -> mailbox queue
Compatibility local endpoint -> mailbox-backed local service endpoint
Network descriptor -> Network Service endpoint plus mailbox-backed descriptor
```

## Reconciliation Against ATX-SPEC-010 Audit Model

ATX-SPEC-015 correctly requires audit records to preserve both:

```text
Compatibility view
Native Atarix authority decision
```

The model also requires native error context to survive compatibility projection.

This prevents POSIX-style simplified return codes from erasing security-relevant audit context.

### Result

```text
Aligned
```

### Follow-Up

Add POSIX compatibility entries to the audit event registry:

```text
POSIX_ENV_CREATED
POSIX_PROFILE_SELECTED
POSIX_PATH_LOOKUP
POSIX_DESCRIPTOR_OPENED
POSIX_DESCRIPTOR_DUPLICATED
POSIX_DESCRIPTOR_INHERITED
POSIX_PROGRAM_LOADED
POSIX_PROCESS_CREATED
POSIX_IPC_EVENT_SENT
POSIX_NETWORK_ENDPOINT_CREATED
POSIX_ERROR_MAPPED
POSIX_RECOVERY_STARTED
POSIX_RECOVERY_RECONCILED
POSIX_ENV_QUARANTINED
```

## Reconciliation Against ATX-SPEC-011 Error Model

ATX-SPEC-015 correctly treats compatibility errors as projections of native Atarix errors.

The native error remains authoritative.

The specification explicitly states that unknown native errors must not be silently converted to success.

### Result

```text
Aligned
```

### Follow-Up

Create a POSIX compatibility error mapping table.

The table should preserve:

```text
Native error code
Compatibility error code
Object or service identity
Operation
Policy result
Lifecycle state
Audit event identity
```

## Reconciliation Against ATX-SPEC-018 Recovery and Reconciliation Model

ATX-SPEC-015 correctly states:

```text
Compatibility recovery restores execution.
Compatibility recovery does not restore authority.
```

Recovery validates descriptors, paths, processes, network endpoints, audit continuity, and quarantine outcomes against native Atarix state.

This is aligned with ATX-SPEC-018's recovery principle that recovery restores trust by proving consistency, not by assuming it.

### Result

```text
Aligned
```

### Follow-Up

Add POSIX recovery tests to the recovery test plan:

```text
Descriptor reconciliation fails closed
Path reconciliation does not grant access
Process reconciliation requires native lifecycle validity
Network reconciliation detects broken endpoints
Audit discontinuity becomes explicit
Quarantine blocks normal execution
Unknown recovery outcome fails closed
```

## Reconciliation Against ATX-SPEC-019 Service Model

ATX-SPEC-015 models device-like endpoints, network endpoints, storage-like endpoints, terminal-like endpoints, and entropy-like endpoints as service-bound compatibility endpoints rather than native device authority.

This is aligned with the Service Model principle that service location is not service identity.

Compatibility endpoint names are bindings to native services.

### Result

```text
Aligned
```

### Follow-Up

Define the initial service binding map:

```text
Null-like endpoint -> Null Service
Entropy-like endpoint -> Entropy Service
Terminal-like endpoint -> Terminal Service
Storage-like endpoint -> Storage Service
Network-like endpoint -> Network Service
```

## Reconciliation Against ATX-SPEC-021 Memory and Data Movement Model

ATX-SPEC-015 defers shared memory and memory mapping semantics to ATX-SPEC-021.

This is the correct architectural placement.

Shared memory must not become a path around capability checks, policy checks, audit, or recovery boundaries.

### Result

```text
Aligned, pending ATX-SPEC-021
```

### Follow-Up

When ATX-SPEC-021 is drafted, include a section for POSIX shared memory projection.

## Findings

### Finding 001: POSIX Compatibility Is Properly Contained

ATX-SPEC-015 consistently treats POSIX as a projection over native Atarix semantics.

This prevents POSIX identity, paths, descriptors, and process identifiers from becoming native authority.

### Finding 002: Mailbox-Centered IPC Is The Correct Native Mapping

ATX-SPEC-015 avoids creating separate native subsystems for POSIX events, streams, queues, local endpoints, and network descriptors.

This keeps authority, audit, recovery, and resource accounting unified.

### Finding 003: Error And Audit Registries Are Now Required

The POSIX model is coherent, but implementation should not begin until compatibility audit events and error mappings are registered.

### Finding 004: Profiles Need Object Format

The profile model is sound, but the profile registry needs a concrete object format before implementation.

### Finding 005: Shared Memory Must Wait For ATX-SPEC-021

The deferment of shared memory semantics is correct.

Implementing compatibility shared memory before ATX-SPEC-021 would risk creating unreviewed authority and data movement behavior.

## Required Follow-Up Work

```text
1. Create POSIX compatibility profile registry object format.
2. Create POSIX compatibility error mapping table.
3. Add POSIX compatibility audit event identifiers.
4. Define compatibility IPC mapping table against ATX-SPEC-005.
5. Define initial service binding map against ATX-SPEC-019.
6. Add POSIX recovery tests against ATX-SPEC-018.
7. Defer shared memory semantics to ATX-SPEC-021.
```

## Review Decision

ATX-SPEC-015 is accepted as coherent with the current architecture baseline.

It should remain marked as draft until the follow-up registries and mapping tables are created.

## Summary

ATX-SPEC-015 successfully preserves the central Atarix architecture rule:

```text
Compatibility is not authority.
```

The specification is ready to inform implementation planning after registries are created for profiles, errors, audit events, IPC mappings, and service bindings.
