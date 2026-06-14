# ATARIX

ATARIX is a modular experimental workstation and operating-system research architecture built around interchangeable CPU cards, a Fabric Northbridge implemented in an ECP5 FPGA, a supervisor management plane, capability-mediated resource access, explicit security doctrine, and a Sun/NuBus/UPA-inspired modular backplane.

The initial Rev A CPU-card target is the WDC W65C816S. That processor is an implementation target, not a permanent architectural constraint. ATARIX is designed so that future CPU cards can be introduced without changing the public system architecture.

The project is intended to support operating-system research, FPGA-based system architecture, secure device mediation, hardware bring-up, heterogeneous compute experiments, and retro-modern workstation design.

## Architecture Doctrine

ATARIX follows a trust-nothing architecture.

Core doctrine:

```text
Authority is explicit.
Trust is explicit.
Ownership is explicit.
Persistence is explicit.
Cleanup is explicit.
System objects are not exempt.
Identity is not authority.
Connectivity is not trust.
Compromise is expected.
Containment is mandatory.
```

The project treats architecture as the product and code as an implementation of that architecture.

All future subsystems are reviewed against the security doctrine and architecture review process documented under:

```text
docs/doctrine/ARC-SEC.md
docs/doctrine/ARC-REVIEW.md
docs/reviews/architecture-review-template.md
```

## Security Objectives

ATARIX is designed to reduce the impact of common exploit classes by separating code execution from authority.

Primary objectives:

- Prevent ambient authority.
- Prevent privilege escalation by identity alone.
- Prevent authority forgery.
- Prevent capability bypass.
- Prevent object spoofing.
- Prevent lateral movement and pivoting.
- Contain compromised components.
- Minimize attack surface.
- Fail closed by default.
- Support revocation.
- Support lifecycle cleanup.
- Support forensic auditing.

A successful exploit against one component should not automatically become control over the system.

## Core Architectural Principles

1. Everything visible to the system is an object.
2. Names are bindings, not identities.
3. Identity is opaque, immutable, globally unique within the fabric, and never reused.
4. Authority is explicit, scoped, auditable, revocable, and bounded.
5. No ambient authority.
6. Connectivity does not imply trust.
7. Code execution does not imply authority.
8. Compromise must be containable.
9. Objects are named, not exposed as raw numeric implementation details.
10. System components are not exempt from policy.
11. Cleanup and revocation are architectural requirements.
12. CPU-card details must not leak into public architecture.
13. Fabric topology must not leak into authority.
14. Human-readable security state is a security feature.
15. Architecture review precedes implementation.

## Object and Namespace Model

ATARIX separates identity, name, authority, location, and implementation.

Conceptually:

```text
human-readable name
    -> namespace binding
    -> opaque object identity
    -> object
    -> ring check
    -> capability check
    -> operation
```

Examples of architectural names:

```text
fabric.cpu.slot0
fabric.memory.ddr0
fabric.storage.boot
fabric.network.eth0
fabric.service.audit
system.directory
system.object-manager
```

Numeric handles, table slots, mailbox numbers, memory addresses, and CPU-local pointers are implementation details. They are not public architectural identifiers.

## Lifecycle and Cleanup

Every object and resource must have:

- Owner
- Scope
- Lifetime
- Cleanup authority
- Persistence policy
- Audit visibility

No object is immortal by default.

No object may become orphaned silently.

Temporary resources should use leases by default.

Persistence must be requested and justified.

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
CPU Card
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
CPU Card
    -> CPU-local SRAM and ROM
    -> Fabric Northbridge Interface
    -> DIN41612-style modular backplane
    -> ECP5 Fabric Northbridge
    -> Service-oriented architecture
    -> Memory cards, accelerators, networking, and future CPU cards
```

The supervisor management plane is separate from the main control/data/event fabric and remains responsible for reset, recovery, RTC, watchdogs, health monitoring, and fault logging.

Supervisor authority is explicit. Supervisor components are not exempt from ownership, lifecycle, audit, cleanup, scope, or persistence rules.

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
L4  CPU-local SRAM
L5  Fabric Memory Services
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

Current public headers should be treated as implementation-facing interfaces and reconciled with the architecture specifications as the numbered ATX-SPEC documents mature.

Known public header areas include:

```text
Discovery
Operations
Rings
Capability records
Capability policy
Mailbox transport
Simulation support
Fabric support
```

## Discovery Subsystem

```text
include/atarix/discovery.h
```

Defines the ATARIX discovery wire format, discovery magic/version constants, discovery record alignment, discovery handles, discovery record types, and discovery flags.

Discovery Sprint 1 established parsing, validation, iteration, duplicate-handle rejection, END-record validation, invalid-length rejection, unknown-record handling, and CRC coverage.

## Operation Registry

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

## Ring Architecture

```text
include/atarix/rings.h
```

Defines ATARIX privilege rings and wire-format mappings.

Rings provide coarse-grained security boundaries.

Capabilities provide fine-grained authority within those boundaries.

Cross-ring authority must be explicit and mediated.

## Capability Model

Capabilities represent explicit authority.

A capability should be:

- Scoped
- Revocable
- Auditable
- Bound to intended authority
- Constrained by ring policy

Possessing an object name or identity does not imply possession of a capability.

Capability behavior is being promoted into:

```text
docs/architecture/ATX-SPEC-003-Capability-Model.md
```

## Mailbox Model

Mailboxes are the architectural transport for inter-object communication.

Mailbox transport does not itself grant authority.

Messages must still be validated against ring policy, object state, and capabilities.

Mailbox behavior is being promoted into:

```text
docs/architecture/ATX-SPEC-005-Mailbox-Model.md
```

## Security Model

ATARIX uses explicit authority and deny-by-default security.

Access decisions are constrained by:

```text
Object identity
    -> Ring policy
    -> Ownership
    -> Capability
    -> Revocation
    -> Expiration
    -> Object lifecycle
    -> Resource policy
    -> Operation
    -> ALLOW / DENY
```

Core security rules:

```text
Authority must never be inferred from execution.
Connectivity does not imply trust.
Compromise is expected and must be containable.
No ambient authority.
Identity theft is not authority theft.
Secure by default.
Fail closed.
System objects are not exempt.
```

The controlling draft is:

```text
docs/architecture/ATX-SPEC-001-Security-Model.md
```

## Current Test Coverage

Current CI baseline:

```text
TOTAL: 40
PASS:  40
SKIP:  0
XFAIL: 0
FAIL:  0
XPASS: 0
ERROR: 0
```

Test inventory:

```text
Discovery tests:   9
Capability tests:  8
Mailbox tests:    10
Simulation tests:  8
Ring tests:        5
Total tests:      40
```

Test coverage currently includes:

- Discovery parsing and validation
- Capability validation, ownership, ring boundaries, delegation, and revocation
- Capability/ring interaction regression tests
- Mailbox CRC, sequencing, validation, duplicate, out-of-order, unknown-type, payload, and ring restrictions
- Simulation node, ping-pong, version, invalid-message, and fabric tests
- Ring self-access, upward denial, downward allowance, quarantine denial, and supervisor override tests

## Architecture Specifications

Numbered architecture specifications:

- [ATX-SPEC-001 Security Model](docs/architecture/ATX-SPEC-001-Security-Model.md)
- [ATX-SPEC-002 Authority Model](docs/architecture/ATX-SPEC-002-Authority-Model.md)
- [ATX-SPEC-003 Capability Model](docs/architecture/ATX-SPEC-003-Capability-Model.md)
- [ATX-SPEC-004 Lifecycle Model](docs/architecture/ATX-SPEC-004-Lifecycle-Model.md)
- [ATX-SPEC-005 Mailbox Model](docs/architecture/ATX-SPEC-005-Mailbox-Model.md)
- [ATX-SPEC-006 Object Model](docs/architecture/ATX-SPEC-006-Object-Model.md)
- [ATX-SPEC-007 Namespace Model](docs/architecture/ATX-SPEC-007-Namespace-Model.md)

Architecture foundation drafts:

- [Architectural Pillars](docs/architecture/pillars.md)
- [Design Principles](docs/architecture/design-principles.md)
- [Object Model](docs/architecture/object-model.md)
- [Namespace Model](docs/architecture/namespace-model.md)
- [Directory Service Architecture](docs/architecture/directory-service.md)
- [Architecture Review Checklist](docs/architecture/architecture-review-checklist.md)

Doctrine and review process:

- [Security Doctrine](docs/doctrine/ARC-SEC.md)
- [Architecture Review Doctrine](docs/doctrine/ARC-REVIEW.md)
- [Architecture Review Template](docs/reviews/architecture-review-template.md)

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

Current architecture workflow:

```text
Discussion
    -> Doctrine / Specification
    -> Architecture Review
    -> Issue
    -> Implementation
    -> Test
    -> CI Green
    -> Merge
```

Before implementation, new work should answer:

```text
Does this create authority?
Does this transfer authority?
Does this persist authority?
Does this expose authority?
Does this create new attack surface?
Does this create objects?
Does this create garbage?
Does this survive reboot?
Does this survive uninstall?
Does this comply with ARC-SEC?
```

If the answer is unclear, update the specification before writing code.

## Architectural Influences

ATARIX draws inspiration from:

- W65C816 homebrew and workstation-class experimentation
- Apple II expansion architecture
- Apple NuBus declaration ROM architecture
- Sun UPA and workstation fabric architecture
- Open Firmware device-tree concepts
- PCI capability discovery concepts
- Vega816 CPU shim, buffering, DMA, and vector-pull-rewrite concepts
- Capability-based security systems
- OpenBSD-style distrust of unsafe assumptions
- Service-processor recovery models
- Lattice ECP5 / ULX3S FPGA development

## Status

Completed:

- Discovery Sprint 1
- Capability Sprint 1
- Ring security model implementation tests
- Mailbox security regression tests
- Simulation fabric tests
- Security doctrine foundation
- Architecture review process foundation
- ATX-SPEC-001 through ATX-SPEC-007 draft structure

Current phase:

```text
Architecture Phase 2: Core System Semantics
```

Upcoming architecture work:

- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model

The current CI baseline is 40/40 tests passing.
