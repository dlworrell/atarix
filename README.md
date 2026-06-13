# ATARIX

ATARIX is a modular experimental workstation architecture built around W65C816 CPU cards, a Fabric Northbridge implemented in an ECP5 FPGA, a supervisor management plane, capability-mediated resource access, and a Sun/NuBus/UPA-inspired modular backplane.

The project is intended to support operating-system research, FPGA-based system architecture, secure device mediation, hardware bring-up, heterogeneous compute experiments, and retro-modern workstation design.

## Fabric Northbridge Architecture

A central architectural concept in ATARIX is the Fabric Northbridge.

The Fabric Northbridge is implemented in the ECP5 FPGA and serves as the integration point between processor cards and the rest of the system.

Responsibilities include:

- Service discovery
- Mailbox transport
- DMA services
- Interrupt routing
- Memory services
- Resource management
- Future multi-processor coordination

Conceptually:

```text
W65C816 CPU Card
        ↕
Fabric Northbridge (ECP5)
        ↕
Memory Services
Storage Services
Network Services
Accelerators
Future CPU Cards
```

The W65C816 is not exposed directly to a shared system bus.

Instead, all system-wide communication occurs through the Fabric Northbridge.

## Current Architectural Direction

ATARIX is not a raw W65C816 bus stretched across a backplane.

The current design direction is:

```text
W65C816 CPU Card
    -> Local SRAM and ROM
    -> Fabric Northbridge Interface
    -> DIN41612-style modular backplane
    -> ECP5 Fabric Northbridge
    -> Service-oriented architecture
    -> Memory cards, accelerators, networking, and future CPU cards
```

The supervisor management plane is separate from the main control/data/event fabric and remains responsible for reset, recovery, RTC, watchdogs, health monitoring, and fault logging.

## Memory Architecture

ATARIX uses a hierarchical memory model.

### CPU-Local Memory

The W65C816 CPU card is expected to contain the maximum directly addressable SRAM supported by the processor.

Target direction:

```text
16 MiB CPU-local SRAM
```

CPU-local SRAM is intended for:

- Kernel execution
- Interrupt handlers
- Direct page
- Stack
- Local buffers
- Deterministic execution

### Fabric Memory

Large system memory resides behind the Fabric Northbridge.

Target direction:

```text
ECC DDR5 Memory Cards
        ↓
Fabric Northbridge Memory Controller
        ↓
Memory Services
```

Fabric memory is intended for:

- File cache
- Framebuffers
- Network buffers
- Shared memory
- Large datasets
- Future processor cards

### Memory Hierarchy

```text
L0  CPU Registers
L1  CPU Cache
L2  CPU Cache
L3  CPU Cache
L4  16 MiB CPU-local SRAM
L5  Fabric Memory Services (ECC DDR5)
L6  Persistent Storage Services
```

## Rev A Baseline

Current Rev A baseline selections:

- **Fabric Northbridge:** ULX3S with Lattice LFE5U-85F ECP5 FPGA
- **Supervisor MCU:** RP2350
- **CPU:** WDC W65C816S
- **Backplane direction:** DIN41612-style modular workstation backplane
- **CPU-card memory:** maximum directly addressable SRAM
- **System memory:** ECC DDR5 memory cards behind the Fabric Northbridge
- **Networking:** early netboot and NTP support, likely using W5500 during bring-up
- **Diagnostics:** extensive test points, logic-analyzer access, LEDs, DIP switches, and recovery controls

See [Rev A Hardware Baseline](docs/rev-a-hardware-baseline.md).

## ATARIX Public Headers

The public C interface is currently organized under:

```text
include/atarix/
```

Current public headers:

```text
include/atarix/discovery.h
include/atarix/operations.h
include/atarix/rings.h
include/atarix/capability.h
include/atarix/capability_engine.h
include/atarix/capability_policy.h
```

### Discovery Subsystem

```text
include/atarix/discovery.h
```

Defines the ATARIX discovery wire format, discovery magic/version constants, discovery record alignment, discovery handles, discovery record types, and discovery flags.

Discovery Sprint 1 established parsing, validation, iteration, duplicate-handle rejection, END-record validation, invalid-length rejection, unknown-record handling, and CRC coverage.

### Operation Registry

```text
include/atarix/operations.h
```

Defines canonical operation identifiers used by discovery records, capability records, policy evaluation, diagnostics, CPU health operations, trace operations, and experimental operation ranges.

Examples:

```text
READ
WRITE
CONTROL
RESET
BOOT
RECOVER
DELEGATE
REVOKE
```

### Ring Architecture

```text
include/atarix/rings.h
```

Defines ATARIX privilege rings and wire-format mappings.

Supported rings:

```text
Supervisor (-2)
Fabric     (-1)
Kernel      (0)
Driver      (1)
Service     (2)
Application (3)
```

### Capability Record Format

```text
include/atarix/capability.h
```

Defines Capability Record v1, capability flags, trust levels, and helper predicates for destructive operations and Supervisor approval requirements.

Capability Record v1 is currently fixed at:

```text
56 bytes
```

Capability flags include:

```text
READ_ALLOWED
WRITE_ALLOWED
CONTROL_ALLOWED
DELEGATION_ALLOWED
REVOCATION_SUPPORTED
TIME_LIMITED
DEVELOPMENT_ONLY
RECOVERY_ONLY
DESTRUCTIVE_OPERATION
REQUIRES_SUPERVISOR_APPROVAL
```

### Capability Engine

```text
include/atarix/capability_engine.h
```

Defines the earlier capability-evaluation interface for matching capability records against a target handle, operation identifier, requester ring, trust level, and request flags.

This interface remains part of the public header inventory and may be reconciled with `capability_policy.h` in a later sprint.

### Capability Policy

```text
include/atarix/capability_policy.h
```

Defines the Capability Sprint 1 policy interface.

The policy API supports:

```text
Capability record validation
Operation allow/deny checks
Delegation validation
Policy request evaluation
Policy result reporting
Revocation request flags
Expiration enforcement
```

Capability Sprint 1 verified the following security invariants:

```text
Validation
Ownership
Ring boundaries
Delegation
Revocation
Expiration
Deny-by-default behavior
```

## Security Model

ATARIX uses a capability-based security architecture.

Access decisions are determined by:

```text
Identity
    -> Trust
    -> Revocation
    -> Expiration
    -> Ring
    -> Ownership
    -> Capability
    -> Resource
    -> Operation
    -> ALLOW / DENY
```

A request must satisfy all applicable constraints before authorization is granted.

Core security rules:

```text
Supervisor is the root of trust.
Fabric is the enforcement authority.
Capabilities are principal-bound.
Capabilities are non-transferable.
Delegation may only reduce authority.
Revocation is authoritative.
Expiration is enforced.
Default decision is DENY.
```

## Current Test Coverage

Current CI baseline:

```text
TOTAL: 14
PASS:  14
FAIL:  0
ERROR: 0
```

Test inventory:

```text
Discovery tests:   9
Capability tests:  5
Total tests:      14
```

Discovery tests:

```text
discovery/test_discovery_header
discovery/test_discovery_crc
discovery/test_discovery_records
discovery/test_discovery_iteration
discovery/test_discovery_parser_minimal
discovery/test_duplicate_handle
discovery/test_missing_end
discovery/test_invalid_length
discovery/test_unknown_record
```

Capability tests:

```text
capability/test_capability_validation
capability/test_capability_ownership
capability/test_capability_ring_boundary
capability/test_capability_delegation
capability/test_capability_revocation
```

## Recent Additions: Capability Sprint 1

Capability Sprint 1 added or hardened:

```text
capability_policy.h
capability_policy.c
Capability validation API
Ownership enforcement
Ring boundary enforcement
Delegation validation
Revocation handling
Expiration handling
Capability policy tests
```

Capability Sprint 1 review:

```text
docs/reviews/capability-sprint-1-review.md
```

## Documentation Index

### System Overview

- [System Specification v1](docs/system-spec-v1.md)
- [Architecture Design Document](docs/design.md)
- [Hardware Architecture](docs/hardware.md)
- [Software Architecture](docs/software.md)
- [Project Roadmap](docs/roadmap.md)
- [Development Philosophy](docs/development-philosophy.md)
- [Architecture Decision Records](docs/adr/README.md)
- [GitHub Project Organization](docs/github-project-organization-v1.md)
- [Capability Sprint 1 Review](docs/reviews/capability-sprint-1-review.md)

### Hardware Architecture

- [Rev A Hardware Baseline](docs/rev-a-hardware-baseline.md)
- [CPU Card Architecture v1](docs/cpu-card-architecture-v1.md)
- [Supervisor Card Architecture v1](docs/supervisor-card-v1.md)
- [Fabric Northbridge Architecture v1](docs/fabric-controller-architecture-v1.md)
- [Boot Firmware and BIOS Architecture v1](docs/boot-firmware-architecture-v1.md)
- [NuBus and UPA Architectural Influences](docs/nubus-upa-influences-v1.md)
- [Fabric Service Model v1](docs/fabric-service-model-v1.md)

## Project Governance

Repository governance and workflow are documented in [GitHub Project Organization v1](docs/github-project-organization-v1.md).

Key workflow:

```text
Discussion
    -> ADR
    -> Issue
    -> Implementation
    -> Test
    -> CI Green
    -> Close Issue
```

## Architectural Influences

ATARIX draws inspiration from:

- W65C816 homebrew and workstation-class experimentation
- Apple II expansion architecture
- Apple NuBus declaration ROM architecture
- Sun UPA and workstation fabric architecture
- Open Firmware device-tree concepts
- PCI capability discovery concepts
- Vega816 CPU shim, buffering, DMA, and vector-pull-rewrite concepts
- Modern capability-based security and service-processor recovery models
- Lattice ECP5 / ULX3S FPGA development

## Core Principles

1. Keep the W65C816 local bus local to the CPU card.
2. Treat the Fabric Northbridge as the architectural center of the system.
3. CPU width is not system width.
4. Service-oriented communication over raw shared buses.
5. Self-describing hardware and discovery.
6. DMA-first transport.
7. Capability-mediated resource access.
8. Extensive observability and diagnostics.
9. Support heterogeneous future processor cards.
10. Separate management, discovery, and transport planes.

## Status

Discovery Sprint 1 and Capability Sprint 1 are complete.

The current CI baseline is 14/14 tests passing.

The project is ready to proceed toward Directory Sprint 1, public API reconciliation, and first cross-subsystem integration work.
