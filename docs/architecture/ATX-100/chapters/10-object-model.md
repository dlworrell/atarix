# Chapter 10: Object Model

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/address-space-architecture.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/address-space-architecture.md -->
### Integrated source: `docs/address-space-architecture.md`

# ATARIX Address Space Architecture

## Status

Draft architectural specification.

This document defines the long-term organization of CPU-local address spaces, fabric resources, memory-service objects, DMA windows, discovery services, mailboxes, future accelerators, and future 32-bit / 64-bit CPU cards.

## Purpose

The earlier memory-map document focuses on Rev A W65C816 bring-up.

This document defines the larger architectural model that future revisions should converge toward.

## Data Model

Address-space definitions follow:

```text
docs/data-model-and-endianness.md
```

All architecture-defined multi-byte fields are little-endian.

The W65C816's u24 address space is a CPU-local address space, not the universal ATARIX address space.

## Design Rules

CPU width is not system width.

CPU-local address width is not fabric address width.

A CPU card may expose its own local address model while the ATARIX fabric uses resource IDs, object IDs, offsets, windows, and capabilities.

## Design Goals

1. Preserve a simple Rev A bring-up path.
2. Maintain compatibility with the W65C816 24-bit address space.
3. Avoid constraining future CPU cards to the W65C816 address model.
4. Support modular hardware.
5. Support DMA mediation.
6. Support capability-based ownership.
7. Permit future MMU experimentation.
8. Permit accelerator integration.
9. Support large memory and storage objects through fabric services.

## Address Classes

ATARIX distinguishes address classes rather than assuming one global CPU address width.

```text
W65C816 native address      u24
Future 32-bit CPU address   u32
Future 64-bit CPU address   u64
Fabric register offset      u32
Fabric resource ID          u64
Memory object ID            u64
Memory object offset        u64
Storage object ID           u64
Storage object offset       u64
Capability ID               u64
```

A u24 address is valid for W65C816 CPU-local access.

It is not a universal system address.

## W65C816 Address Space

The W65C816 provides:

```text
24-bit addressing
256 banks
64 KiB per bank
16 MiB total native address space
```

ATARIX treats the W65C816 address space as the local address space of a CPU card.

Future CPU cards may expose different local address widths without changing the fabric object model.

## System Address Model

ATARIX should be modeled as a fabric of resources rather than as a single flat W65C816 memory map.

A general fabric reference should be represented conceptually as:

```text
Resource ID: u64
Offset:      u64
Length:      u32 or u64, depending on resource class
Capability:  u64
```

A CPU card may map part of a resource into a local CPU-visible aperture.

For the W65C816, that aperture may be a 16 KiB, 32 KiB, or 64 KiB window inside the 24-bit local address space.

For a future 32-bit or 64-bit CPU card, the aperture may be much larger.

## W65C816 Architectural Regions

The following regions describe the W65C816 CPU-card local view, not the entire ATARIX system.

```text
Region 0  Boot and Core Services
Region 1  General CPU-Local System Memory
Region 2  Device and I/O Space
Region 3  Fabric Services Aperture
Region 4  DMA Windows
Region 5  Discovery Space
Region 6  Accelerator Windows
Region 7  Reserved and Experimental
```

## Region 0 - Boot and Core Services

Primary bank:

```text
Bank $00
```

Contains:

```text
Direct Page
Stack
Monitor RAM
Boot Services
I/O Aperture
ROM
Interrupt Vectors
```

This region must remain usable even when advanced fabric services are unavailable.

## Region 1 - General CPU-Local System Memory

Candidate range:

```text
$010000-$7FFFFF
```

Purpose:

```text
Kernel Memory
Application Memory
Buffers
File Cache
Shared Regions
```

This region is the primary W65C816-local memory pool for software execution.

It is not the maximum possible ATARIX system memory.

Large memory is exposed through memory-service objects and windows.

## Region 2 - Device and I/O Space

Candidate range:

```text
$800000-$8FFFFF
```

Purpose:

```text
UART
Network Controllers
Storage Controllers
GPIO
Legacy Device Registers
```

Device register windows should be explicitly discoverable.

## Region 3 - Fabric Services Aperture

Candidate range:

```text
$900000-$9FFFFF
```

Purpose:

```text
Interrupt Controller
Mailbox Engine
DMA Engine
Timer Services
Fabric Diagnostics
Capability Services
```

This region becomes the primary W65C816 software interface to FPGA-mediated system services.

The fabric itself may manage resources larger than this aperture.

## Region 4 - DMA Windows

Candidate range:

```text
$A00000-$BFFFFF
```

Purpose:

```text
DMA Buffers
High-Speed Transfers
Network Buffers
Storage Buffers
Accelerator Buffers
Mapped Memory-Service Windows
```

Access should be governed by capability grants.

DMA visibility does not imply unrestricted ownership.

## Region 5 - Discovery Space

Candidate range:

```text
$C00000-$CFFFFF
```

Purpose:

```text
Discovery ROMs
Slot Discovery Records
Device Identity Tables
Supervisor Discovery Records
```

Discovery information should be readable without enabling advanced services.

See:

```text
docs/discovery-rom-format.md
```

## Region 6 - Accelerator Windows

Candidate range:

```text
$D00000-$EFFFFF
```

Purpose:

```text
ARM Compute Modules
FPGA Accelerators
Future Heterogeneous Devices
Mapped Accelerator Command Windows
```

Accelerator access should be mediated through mailbox and capability mechanisms.

Accelerators should not receive unrestricted memory visibility.

## Region 7 - Reserved and Experimental

Candidate range:

```text
$F00000-$FFFFFF
```

Potential uses:

```text
Debug Services
Supervisor Diagnostics
Experimental MMU Research
Alternate Firmware Images
Future Expansion
```

## Memory Service Objects

Memory services expose memory as objects, not as mandatory extensions of the W65C816 native address space.

A memory object should use:

```text
Memory Object ID: u64
Object Size:      u64
Object Offset:    u64
Capability ID:    u64
```

A W65C816 CPU card accesses a memory object through a mapped window.

Example:

```text
Memory Object #42
Offset: 0x0000000200000000
Window: $A00000-$A0FFFF
Length: 64 KiB
```

A future 64-bit CPU card may map a larger portion of the same object without changing the object itself.

## Storage Service Objects

Storage services should use the same large-object concept.

```text
Storage Object ID: u64
Extent Offset:     u64
Extent Length:     u64
Capability ID:     u64
```

Storage objects should not be constrained by CPU-local address width.

## Window Mapping

Window mapping is the process of exposing a portion of a fabric resource or memory object into a CPU-local aperture.

A mapping requires:

```text
Resource or Object ID
Offset
Length
Target CPU-local aperture
Capability grant
Access rights
```

The target aperture width depends on the CPU card.

For W65C816:

```text
Target CPU address: u24
Window length:      typically u16 or u32
```

For future 32-bit or 64-bit processors:

```text
Target CPU address: u32 or u64
Window length:      u32 or u64
```

## Ownership Model

Every region, object, or window may have:

```text
Owner
Permissions
Visibility Rules
Capability Requirements
```

Ownership is a logical concept and may be enforced through software, FPGA services, or future hardware assistance.

## Shared Memory

Shared memory regions should be explicitly declared.

Properties:

```text
Owner
Participants
Access Rights
Lifetime
Capability IDs
```

Shared memory should not automatically become globally visible.

## Mailbox Memory

Mailbox payload storage may reside in:

```text
Dedicated Mailbox RAM
Shared System RAM
Fabric-Owned Memory
Memory-Service Object
```

Mailbox ownership remains governed by mailbox and capability rules.

See:

```text
docs/mailbox-protocol-v1.md
```

## DMA Visibility

DMA-capable devices must possess:

```text
DMA Capability
Source Resource Capability
Destination Resource Capability
Valid Ownership Chain
```

DMA may operate on CPU-local addresses or fabric resources depending on descriptor class.

See:

```text
docs/dma-engine-v1.md
```

## Future MMU Experiments

Future revisions may introduce:

```text
Address Translation
Process Isolation
Window Mapping
Capability-Backed Address Spaces
Per-CPU Local Address Spaces
Fabric Resource Translation
```

This specification intentionally avoids requiring an MMU.

## Discovery Integration

Discovery records may describe:

```text
CPU-Local Memory Windows
Fabric Resource Windows
I/O Windows
DMA Regions
Mailbox Endpoints
Accelerator Regions
Memory Service Objects
Storage Service Objects
```

See:

```text
docs/discovery-rom-format.md
```

## Design Rules Summary

1. Region ownership is explicit.
2. DMA visibility does not imply authority.
3. Discovery remains available early in boot.
4. Boot services remain independent of advanced fabric functions.
5. Future MMU work must preserve compatibility with existing firmware where practical.
6. W65C816 u24 addresses describe CPU-local access, not universal system addressing.
7. Fabric resources use resource IDs, object IDs, offsets, windows, and capabilities.
8. Future 32-bit and 64-bit CPU cards must not be forced into the W65C816 address model.

## Open Questions

- Final W65C816-local region boundaries.
- Whether regions are fixed or configurable.
- Supervisor visibility rules.
- Shared-memory implementation details.
- Future MMU architecture.
- Accelerator memory-coherency strategy.
- Whether capability tables reside in dedicated fabric memory.
- Whether memory-service objects require 128-bit identifiers in future distributed systems.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0012-abstraction-and-compatibility.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0012-abstraction-and-compatibility.md -->
### Integrated source: `docs/adr/ADR-0012-abstraction-and-compatibility.md`

# ADR-0012: Abstraction and Compatibility

Status: Accepted

Date: 2026-06-10

## Context

ATARIX is expected to evolve across multiple CPU cards, FPGA revisions, memory technologies, storage technologies, supervisor implementations, and accelerator architectures.

If higher-level software depends on implementation details, the system will become difficult to extend or replace.

## Decision

ATARIX shall define stable architectural contracts and keep implementations replaceable.

Architectural contracts include:

- Mailbox semantics.
- DMA semantics.
- Discovery records.
- Capability records.
- Memory object model.
- Supervisor service model.
- Register access semantics.
- Boot image formats.
- Test formats.

Specific implementations of those contracts may change over time.

## Evidence

The project has already separated CPU-local addressing from fabric resources, native DMA descriptors from fabric DMA descriptors, identity EEPROMs from discovery records, and discovery from authority.

Those separations reduce coupling and preserve future compatibility.

## Alternatives Considered

- Optimize every interface for the first W65C816 implementation.
- Expose FPGA internals as architectural requirements.
- Treat fixed CPU addresses as the primary system abstraction.
- Delay abstraction until a later revision.

## Consequences

Pros:

- Future CPU cards can be added.
- Future FPGA families can be used.
- Memory and storage technologies can change.
- System-level tests remain valid across implementations.
- Software can target architectural behavior rather than board-specific details.

Cons:

- More documentation is required.
- Some implementation-specific optimizations may need wrapper interfaces.
- Initial designs may be slightly more verbose.

## Future Revisions

Future ADRs may refine individual contracts, but the project should continue to prefer stable interfaces over exposed implementation details.

## Design Rule

Implementations may change.

Contracts should remain stable.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/object-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/object-model.md -->
### Integrated source: `docs/architecture/object-model.md`

# Atarix Object Model

## Purpose

The Object Model defines the fundamental unit of identity, authority, naming, ownership, communication, and resource management within Atarix.

All externally visible entities within Atarix are represented as objects.

## Definition

An object is a uniquely identifiable entity that participates in the Atarix namespace and may possess ownership, authority, capabilities, resources, and communication endpoints.

## Object Identity

An object's identity:

- Exists independently of its name.
- Exists independently of its location.
- Exists independently of its capabilities.
- Is opaque.
- Is immutable.
- Is globally unique within the Atarix fabric.
- Is never reused.

Identity shall never be inferred from physical location, ring number, table slot, local handle, memory address, or implementation-specific layout.

## Object Naming

Objects may possess one or more human-readable names. Names are bindings, not identities.

Examples:

```text
system.directory
system.object-manager
service.discovery
device.serial0
storage.boot
fabric.node.7
```

## Required Properties

Every object has:

- Identity
- Type
- Owner
- Ring association
- Lifecycle state
- Resource accounting
- Audit visibility

## Invariants

1. Every object possesses exactly one identity.
2. Identity is opaque and immutable.
3. Identity is not a name.
4. Identity is not a mailbox.
5. Identity is not a capability.
6. Identity is not a memory address.
7. Identity is not a table slot, index, or local handle.
8. Every object possesses exactly one owner.
9. Every object belongs to exactly one ring.
10. Authority must be traceable.

<!-- AEMS-MIGRATED-SOURCE: docs/toolchain.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/toolchain.md -->
### Integrated source: `docs/toolchain.md`

# ATARIX Toolchain Strategy

## Goals

Use open and maintainable tools wherever practical.

The toolchain should support firmware, operating-system development, FPGA development, simulation, and automated testing.

## Assembly Tools

Candidate tools:

- 64TASS
- WDC tools

## FPGA Tools

Primary path:

- Yosys
- nextpnr
- Project Trellis

Target platform:

- Lattice ECP5
- ULX3S

## Editors and IDEs

Candidate tools:

- Vim
- VS Code
- Foenix IDE

## Build System

Candidate tools:

- Make
- CMake research path
- CI automation

## Simulation

Candidate tools:

- Verilator
- Icarus Verilog
- Custom W65C816 emulation

## Firmware Development

Stages:

- ROM monitor
- Diagnostics
- Loader
- Kernel

## Long-Term Objectives

- Fully reproducible builds
- Automated regression testing
- FPGA simulation workflows
- Cross-platform development support
