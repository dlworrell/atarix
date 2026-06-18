# ADR: Three Fabric Architecture

Status: Accepted

## Context

Traditional systems often combine management, enumeration, and data transport into a single bus architecture.

This creates coupling between:

- platform health
- device discovery
- resource authorization
- bulk data movement
- recovery operations

ATARIX is intended to support multiple processor architectures, long service life, integrated instrumentation, self-validation, and recovery independent of the operating system.

A more explicit separation of concerns is required.

## Decision

ATARIX adopts a Three Fabric Architecture.

```text
Management Fabric
Enumeration Fabric
Transport Fabric
```

Each fabric has distinct responsibilities and authority.

## Management Fabric

The Management Fabric is governed by the supervisor.

Primary authority:

```text
RP2350 Supervisor
```

Responsibilities include:

```text
Identity validation
Health monitoring
Telemetry
Provisioning
Recovery
Reset control
Fault logging
Policy enforcement
Instrumentation management
```

Primary question answered:

```text
Is the platform healthy and allowed to operate?
```

The Management Fabric remains available as much as practical during failure and recovery scenarios.

## Enumeration Fabric

The Enumeration Fabric describes the platform.

Primary authority:

```text
Fabric Controller
```

Responsibilities include:

```text
Identity records
Discovery records
Resource records
Service records
Capability descriptions
Provider descriptions
Version descriptions
Binding information
```

Primary questions answered:

```text
What exists?
What services exist?
What versions exist?
How do I bind to them?
```

The Enumeration Fabric describes resources and services.

It does not grant authority.

## Transport Fabric

The Transport Fabric moves information.

Responsibilities include:

```text
Mailbox traffic
DMA traffic
Interrupt signaling
Shared memory transport
Network transport
Bulk data movement
```

Primary question answered:

```text
How do I use the platform?
```

The Transport Fabric is optimized for operation rather than description or governance.

## Relationship Between Fabrics

The fabrics operate together but remain logically distinct.

```text
Management Fabric
    validates
    monitors
    governs

Enumeration Fabric
    identifies
    describes
    binds

Transport Fabric
    moves
    signals
    exchanges
```

## Boot Sequence Model

A typical startup flow is:

```text
Power On
    ↓
Supervisor Initialization
    ↓
Identity Validation
    ↓
Enumeration
    ↓
Capability Evaluation
    ↓
Service Binding
    ↓
Transport Activation
    ↓
Monitor
    ↓
Operating System
```

## Relationship to Identity, Trust, and Capability

The Three Fabric Architecture works together with the Identity/Trust/Capability model.

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Transport operates.
```

Trust evaluation belongs to Management Fabric policy.

Resource and service description belong to the Enumeration Fabric.

Operational communication belongs to the Transport Fabric.

## Instrumentation Integration

Instrumentation services span all three fabrics.

Examples:

```text
Logic Analyzer
Protocol Decoder
Pattern Generator
Counter Bank
DSP Analysis
Trigger Router
```

Instrumentation remains under Management Fabric authority even when exposed through Enumeration and Transport mechanisms.

## Architectural Benefits

This separation:

- reduces coupling
- improves recovery behavior
- improves observability
- supports multiple CPU architectures
- simplifies future expansion
- supports homebrew and experimental hardware
- supports integrated testbench functionality

## Consequences

A CPU card does not require direct knowledge of:

- slot numbering
- physical wiring
- transport implementation details
- fabric topology details

The platform exposes services and resources through enumeration and transport abstractions.

## Related ADRs

- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md

## Related Specifications

- identity-eeprom-v1.md
- supervisor-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- discovery-rom-format-v1.md
- capability-record-format-v1.md
- service-directory-format-v1.md

## Guiding Principle

```text
Management Fabric
    validates and governs

Enumeration Fabric
    describes and binds

Transport Fabric
    transfers and signals
```
