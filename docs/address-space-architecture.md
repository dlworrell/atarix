# ATARIX Address Space Architecture

## Status

Draft architectural specification.

This document defines the long-term organization of the W65C816 24-bit address space and how memory, devices, DMA, discovery services, mailboxes, fabric resources, and future accelerators coexist.

## Purpose

The earlier memory-map document focuses on Rev A bring-up.

This document defines the larger architectural model that future revisions should converge toward.

## Design Goals

1. Preserve a simple Rev A bring-up path.
2. Maintain compatibility with the W65C816 24-bit address space.
3. Support modular hardware.
4. Support DMA mediation.
5. Support capability-based ownership.
6. Permit future MMU experimentation.
7. Permit accelerator integration.

## W65C816 Address Space

The W65C816 provides:

```text
24-bit addressing
256 banks
64 KiB per bank
16 MiB total address space
```

ATARIX treats the address space as a collection of architectural regions rather than merely a flat collection of banks.

## Architectural Regions

```text
Region 0  Boot and Core Services
Region 1  General System Memory
Region 2  Device and I/O Space
Region 3  Fabric Services
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

## Region 1 - General System Memory

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

This region is the primary memory pool for software execution.

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

## Region 3 - Fabric Services

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

This region becomes the primary software interface to FPGA-mediated system services.

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

## Ownership Model

Every region may have:

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
```

Shared memory should not automatically become globally visible.

## Mailbox Memory

Mailbox payload storage may reside in:

```text
Dedicated Mailbox RAM
Shared System RAM
Fabric-Owned Memory
```

Mailbox ownership remains governed by mailbox and capability rules.

## DMA Visibility

DMA-capable devices must possess:

```text
DMA Capability
Memory Capability
Valid Ownership Chain
```

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
```

This specification intentionally avoids requiring an MMU.

## Discovery Integration

Discovery records may describe:

```text
Memory Windows
I/O Windows
DMA Regions
Mailbox Endpoints
Accelerator Regions
```

See:

```text
docs/discovery-rom-format.md
```

## Design Rules

1. Region ownership is explicit.
2. DMA visibility does not imply authority.
3. Discovery remains available early in boot.
4. Boot services remain independent of advanced fabric functions.
5. Future MMU work must preserve compatibility with existing firmware where practical.

## Open Questions

- Final region boundaries.
- Whether regions are fixed or configurable.
- Supervisor visibility rules.
- Shared-memory implementation details.
- Future MMU architecture.
- Accelerator memory-coherency strategy.
- Whether capability tables reside in dedicated fabric memory.