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