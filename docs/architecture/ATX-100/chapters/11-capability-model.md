# Chapter 11: Capability Model

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0013-resource-oriented-architecture.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0013-resource-oriented-architecture.md -->
### Integrated source: `docs/adr/ADR-0013-resource-oriented-architecture.md`

# ADR-0013: Resource-Oriented Architecture

Status: Accepted

Date: 2026-06-10

## Context

Early revisions of ATARIX are based on a W65C816 CPU card, but the long-term architecture includes memory services, storage services, accelerators, multiple processor types, DMA services, and capability-mediated access.

A fixed-address model centered on a single CPU address space does not scale well to those goals.

## Decision

The primary ATARIX abstraction is a resource.

Resources are represented through identifiers, offsets, windows, services, and capabilities.

CPU-local addresses remain valid within a CPU card but are not the universal architectural abstraction.

## Resource Examples

- Memory objects.
- Storage objects.
- Mailbox endpoints.
- DMA channels.
- Interrupt sources.
- Device windows.
- Accelerator queues.
- Discovery records.
- Supervisor services.

## Evidence

The architecture already uses:

- u64 resource identifiers.
- u64 object identifiers.
- Capability-mediated access.
- Window mapping.
- Fabric descriptors.

## Alternatives Considered

- Flat memory-map-centric architecture.
- CPU-address-centric architecture.
- Per-device ad hoc addressing schemes.

## Consequences

Pros:

- Easier support for future CPU cards.
- Easier support for large memory systems.
- Cleaner DMA architecture.
- Better compatibility with capability-based security.
- Supports distributed and heterogeneous resources.

Cons:

- Additional indirection.
- More metadata and discovery infrastructure.

## Design Rule

Resources, objects, services, and capabilities are primary.

Fixed CPU memory addresses are implementation details of a particular processor card.

<!-- AEMS-MIGRATED-SOURCE: docs/backplane-spec.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/backplane-spec.md -->
### Integrated source: `docs/backplane-spec.md`

# ATARIX Backplane Specification

## Purpose

This document defines the first-pass active-backplane architecture for ATARIX.

The backplane is intended to support modular CPU cards, service modules, FPGA-mediated routing, and future multiprocessor experimentation.

## Design Goals

1. Support a single CPU system initially.
2. Permit future multi-CPU experiments.
3. Avoid requiring every slot to observe every signal.
4. Allow FPGA-assisted arbitration.
5. Support recovery and fault isolation.

## Conceptual Slot Types

### CPU Slot

Hosts:

- W65C816 CPU card
- Future experimental processor modules

### Service Slot

Hosts:

- Networking
- Storage
- Supervisor functions
- Debug hardware

### Fabric Slot

Hosts:

- FPGA fabric modules
- DMA services
- Discovery services

### Accelerator Slot

Hosts:

- ARM modules
- FPGA accelerators
- Future heterogeneous compute devices

## Logical Signal Groups

### Power

Candidate rails:

- +3.3V
- +5V
- Ground
- Auxiliary standby power

### Clocking

Signals:

- System clock
- Reference clock
- Debug clock
- Clock-enable support

### Reset

Signals:

- Global reset
- Slot reset
- Recovery reset

### Interrupts

Candidate structure:

- Interrupt request lines
- Fabric-mediated interrupt routing
- Prioritized delivery

### Mailboxes

Mailbox services support:

- CPU-to-CPU communication
- CPU-to-device communication
- Supervisor messaging

### Discovery

Each slot should expose:

- Vendor identifier
- Device identifier
- Capability description
- Firmware version

## Arbitration Model

Early revisions may use centralized arbitration.

Future revisions may support:

- Bus ownership tracking
- Mailbox ownership
- DMA ownership
- Capability-controlled access

## DMA Considerations

DMA should eventually be mediated through the FPGA fabric.

The fabric should be capable of:

- Granting DMA rights
- Revoking DMA rights
- Detecting violations
- Logging faults

## Recovery Requirements

A failed module should not require complete system replacement.

Desired capabilities:

- Slot isolation
- Supervisor diagnostics
- Independent reset control
- Recovery firmware support

## Open Questions

- Connector family
- Pin count
- Mechanical card dimensions
- Maximum slot count
- Voltage distribution strategy
- Whether CPU slots are electrically identical to accelerator slots
- Fabric topology for Rev C and beyond

<!-- AEMS-MIGRATED-SOURCE: docs/data-model-and-endianness-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/data-model-and-endianness-v1.md -->
### Integrated source: `docs/data-model-and-endianness-v1.md`

# Data Model and Endianness v1

## Purpose

This document defines the canonical ATARIX data model used by firmware, Discovery ROMs, Service Directory records, mailbox packets, DMA descriptors, Boot Information Blocks, Fabric CSRs, and future processor cards.

This document exists to prevent accidental coupling between the W65C816 CPU-local view of the machine and the wider Fabric Northbridge view of the system.

## Core Rules

```text
All ATARIX-defined binary structures are little-endian.

CPU Width != Fabric Width.
Address != Handle.
CPU-local SRAM is for execution.
CPU-local DMA memory is for exchange.
```

These rules apply unless a later specification explicitly states otherwise.

## Endianness

ATARIX structures are little-endian.

This includes:

- Discovery ROM records
- Identity EEPROM records
- capability records
- Service Directory records
- Boot Information Blocks
- mailbox headers
- mailbox payload descriptors
- DMA descriptors
- Fabric CSR fields
- kernel image headers
- monitor records
- provisioning records

The W65C816 is little-endian, and keeping the full architecture little-endian avoids unnecessary conversion in firmware and early bring-up code.

Future processor cards that use a different native byte order must adapt at the CPU-card boundary and present ATARIX-defined structures in little-endian form.

## CPU-Local Address Model

The W65C816 CPU-local address model is 24-bit.

Use type name:

```text
atarix_u24_cpu_addr
```

A CPU-local address refers only to memory visible in the local W65C816 CPU-card address space.

CPU-local addresses must not be used as Fabric addresses, service handles, DMA handles, or global resource identifiers.

## Fabric Address Model

Fabric addresses are 64-bit.

Use type name:

```text
atarix_u64_fabric_addr
```

A Fabric address refers to a fabric-visible memory or resource address, not a W65C816 CPU-local address.

The W65C816 does not directly expose or directly address the complete fabric address space.

## Resource Handles

Resource handles are 64-bit opaque identifiers.

Use type name:

```text
atarix_u64_resource_handle
```

A resource handle is not an address.

Handles may identify:

- services
- devices
- DMA buffers
- mailboxes
- capabilities
- boot resources
- provisioning records
- diagnostic streams

Software must not infer physical address layout from a handle value.

## Service Handles

Service handles are 64-bit opaque identifiers.

Use type name:

```text
atarix_u64_service_handle
```

The Service Directory maps discovered services to service handles. Operating systems and firmware components should bind to service handles rather than physical slots or fixed addresses.

## DMA Handles

DMA handles are 64-bit opaque identifiers.

Use type name:

```text
atarix_u64_dma_handle
```

A DMA handle represents an authorized DMA buffer, window, or transaction context.

DMA handles should be capability-checked and should not be interpreted as raw addresses.

## CPU-Local DMA Boundary

CPU-local DMA memory is the translation boundary between the W65C816 CPU-local world and the Fabric Northbridge world.

```text
W65C816 CPU-local address
    -> CPU-local DMA buffer
    -> CPU-card DMA shim
    -> Fabric address / service handle / DMA handle
```

The Fabric Northbridge should not DMA directly into arbitrary CPU-local SRAM.

## Alignment

Fabric-visible structures should be naturally aligned for 64-bit transport unless a compact ROM format explicitly requires otherwise.

Recommended defaults:

```text
16-bit fields  aligned to 2 bytes
32-bit fields  aligned to 4 bytes
64-bit fields  aligned to 8 bytes
records        aligned to 8 bytes where practical
```

Discovery ROM and Identity EEPROM formats may use compact layouts, but must define alignment explicitly.

## Integer Type Names

Preferred architectural type names:

```text
atarix_u8
atarix_u16
atarix_u24_cpu_addr
atarix_u32
atarix_u64
atarix_u64_fabric_addr
atarix_u64_resource_handle
atarix_u64_service_handle
atarix_u64_dma_handle
```

These names should be reflected in future C headers under `include/atarix/`.

## Binary Format Policy

New binary specifications should explicitly state:

- endianness
- field width
- alignment
- record version
- checksum or integrity mechanism
- whether a value is an address or a handle
- whether an address is CPU-local or fabric-visible

No binary format should use an unqualified word such as `address` when `cpu_local_address`, `fabric_address`, or `handle` is meant.

## Future Processor Cards

ATARIX must support future 32-bit and 64-bit processor cards without changing resource interfaces.

Therefore:

- service handles remain 64-bit
- DMA handles remain 64-bit
- fabric addresses remain 64-bit
- binary structures remain little-endian
- CPU-specific address sizes remain local to each CPU card

## Design Consequence

The W65C816 is a first-class ATARIX CPU node, but it does not define the width of the whole machine.

The Fabric Northbridge defines the system-level transport and resource model.

<!-- AEMS-MIGRATED-SOURCE: docs/design-rationale-history-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design-rationale-history-v1.md -->
### Integrated source: `docs/design-rationale-history-v1.md`

# ATARIX Design Rationale History v1

## Purpose

This document preserves the discussion and reasoning behind the current ATARIX architectural direction.

It is not an implementation specification. Its job is to explain why the project moved from the early seed design toward the current Fabric Northbridge, service-directory, discovery-ROM, hybrid-firmware, and CPU-local DMA architecture.

## Source Material Accounted For

This rationale accounts for:

- the seed design documentation archive uploaded as `atarix_seed_design_docs.zip`
- the June 2026 discussion-output summary uploaded as `atarix_discussion_outputs.zip`
- the current repository documentation inspected during June 2026 updates
- the later design discussions around toolchain, firmware handoff, Discovery ROMs, Service Directory, mailbox transport, and CPU-local DMA memory

The uploaded seed archive contained early design notes for:

- architecture
- backplane
- FPGA fabric
- hardware roadmap
- references
- security model
- Rev A discrete W65C816 board
- Rev B backplane
- Rev C FPGA fabric
- M548 chassis concept
- BIOS / ROM monitor
- kernel notes
- BB816, Vega816, and Flipper One references

The uploaded discussion-output summary recorded discussion results for:

- boot firmware architecture
- NuBus and UPA influences
- fabric service model
- Fabric Northbridge architecture
- README updates emphasizing the Fabric Northbridge
- documentation validation tooling

## Seed Architecture

The early ATARIX seed design described a modular W65C816 experimental workstation or cyberdeck platform.

The initial revision strategy was staged:

```text
Rev A  Discrete W65C816 bring-up board
Rev B  Modular CPU-card/backplane architecture
Rev C  FPGA added as programmable chipset / smart glue
Rev D  FPGA becomes Ring 0 fabric
Rev E  Secure heterogeneous accelerator expansion
```

The key early rule was:

```text
Plugged in does not mean trusted.
```

This rule drove the later emphasis on declared, enumerated, permissioned, and sandboxed hardware.

## Why the Design Moved Beyond a Raw Backplane

The original staged design could have remained a conventional shared-bus 65C816 backplane.

That approach was rejected as the long-term architecture because it would make several goals harder:

- heterogeneous CPU cards
- capability-mediated resource access
- secure DMA
- isolated accelerators
- fabric memory services
- deterministic CPU-local execution
- service-oriented drivers
- reliable discovery and recovery

The current direction keeps the W65C816 local bus local to the CPU card and moves system integration into the Fabric Northbridge.

## Fabric Northbridge Rationale

The Fabric Northbridge became the architectural center because it gives ATARIX a clean separation between CPU-local behavior and system-wide resources.

The W65C816 remains a valid processor node, but it is not forced to behave like a modern shared-memory bus master across the whole machine.

The Fabric Northbridge is responsible for:

- service discovery
- mailbox transport
- DMA coordination
- interrupt routing
- memory services
- resource management
- future multi-processor coordination

This preserves retro-computing compatibility at the CPU-card level while allowing the larger machine to behave more like a modern modular workstation.

## NuBus and UPA Influence

NuBus influenced the self-describing card model.

The key lesson taken from NuBus is not electrical compatibility. It is the architectural idea that expansion cards should describe themselves and their resources so software does not bind blindly to slot numbers.

Sun UPA influenced the fabric/service direction.

The key lesson taken from UPA is not direct protocol cloning. It is the idea that a workstation-class machine benefits from a high-level interconnect model that can mediate traffic between processors, memory, I/O, and graphics-class devices.

The resulting ATARIX principle is:

```text
Bind to services, not slots.
```

## Three-Plane Model

The discussion-output summary recorded a three-plane architecture:

```text
Management Plane   RP2350 supervisor
Discovery Plane    enumeration, declaration, service registration
Data Plane         mailbox, DMA, memory, I/O transport
```

This separation prevents the supervisor MCU, discovery machinery, and data movement fabric from becoming one confused mechanism.

The supervisor manages reset, recovery, health monitoring, fault logging, watchdogs, and board-management functions.

The discovery plane enumerates cards and constructs system knowledge.

The data plane moves commands and payloads between services.

## Service-Oriented Architecture

ATARIX deliberately moved away from slot-oriented drivers.

A slot-oriented model would require software to know where hardware lives.

A service-oriented model lets firmware discover hardware, build a Service Directory, and let the kernel bind to capabilities.

This is important because ATARIX is intended to support:

- memory cards
- network cards
- storage services
- accelerators
- framebuffer services
- future CPU cards
- supervisor services

The Service Directory becomes the handoff object that turns discovered hardware into usable software-facing services.

## Discovery ROM Rationale

Discovery ROMs exist to preserve self-describing hardware.

The system should not depend on a hard-coded assumption that slot 3 is a network card or that a fixed address always contains a storage controller.

Each intelligent card should describe:

- vendor identity
- device identity
- class
- revision
- capabilities
- resources
- service endpoints
- DMA requirements
- interrupt requirements
- diagnostics

This supports both Rev A bring-up and later heterogeneous expansion.

## Hybrid Firmware Model Rationale

Three firmware models were considered.

### Traditional 65xx BIOS

In this model, software continues to call firmware services after boot.

Benefits:

- easy bring-up
- familiar 8-bit/16-bit monitor style
- convenient debugging

Costs:

- firmware can become an operating-system dependency
- harder to build a clean kernel ownership model
- can trap the system in a retro BIOS design

### Open Firmware Style

In this model, firmware discovers hardware and hands the machine completely to the kernel.

Benefits:

- cleaner workstation-style ownership
- kernel controls runtime policy
- better long-term architecture

Costs:

- harder early bring-up
- less convenient recovery and debugging
- more work before the machine becomes usable

### Hybrid Model

ATARIX chooses the hybrid model.

Firmware discovers hardware, builds the Service Directory and Boot Information Block, then hands ownership to the kernel.

A small emergency BIOS remains available for:

- console
- diagnostics
- recovery boot
- firmware update
- supervisor access

This keeps early bring-up practical without making ordinary runtime I/O dependent on BIOS calls.

## Toolchain Rationale

The project should not require Linux on the W65C816 target.

Linux, BSD, macOS, or another modern system may be used as the host development environment, but the target should remain an ATARIX machine.

64TASS is the practical near-term assembler path for W65C816 firmware and monitor work.

WDC tools remain useful as a vendor reference and compatibility check.

A freestanding C subset should come before any full libc.

This avoids blocking firmware bring-up on a complete operating system or hosted C runtime.

## Emulator-First Rationale

The emulator should model the architectural contract before all hardware exists.

It does not need to be cycle-accurate at first.

It should model:

- reset
- ROM loading
- CPU-local SRAM
- CPU-local DMA memory
- mailbox transport
- Fabric Northbridge stubs
- Service Directory
- Boot Information Block

The reason is simple: the firmware, kernel, and FPGA contracts need a test oracle before board bring-up becomes expensive.

## CPU-Local SRAM Rationale

The W65C816 CPU card should contain maximum directly addressable local SRAM, with a 16 MiB target.

CPU-local SRAM is for:

- kernel execution
- interrupt handlers
- direct page
- stack
- local deterministic buffers

This keeps the CPU card useful and deterministic even though larger system memory lives behind the Fabric Northbridge.

## Fabric Memory Rationale

Large system memory belongs behind the Fabric Northbridge.

Fabric memory is for:

- file cache
- framebuffers
- network buffers
- shared memory
- large datasets
- future processor cards

This avoids pretending that the W65C816 local bus is the same thing as the entire workstation memory system.

## CPU-Local DMA Memory Rationale

CPU-local DMA memory was added as an explicit staging and isolation layer.

The Fabric Northbridge should not DMA directly into arbitrary W65C816 execution memory.

Instead:

```text
W65C816 CPU
   -> CPU-local SRAM / ROM
   -> CPU-local DMA buffer memory
   -> CPU-card DMA shim
   -> Fabric Northbridge mailbox and DMA interface
   -> system services
```

The governing rule is:

```text
CPU-local SRAM is for execution.
CPU-local DMA memory is for exchange.
```

This provides:

- bounce buffering
- bus-width adaptation
- timing decoupling
- protection boundary
- deterministic CPU-local SRAM behavior
- a clean service boundary for the Fabric Northbridge

The CPU card advertises CPU-local DMA capability through the Service Directory, and mailbox messages should carry DMA descriptors rather than bulk payloads where practical.

## Security Rationale

The early seed security model treated optional hardware as untrusted until declared, enumerated, permissioned, and sandboxed.

That principle remains central.

The current architecture supports it by combining:

- supervisor management
- discovery ROMs
- service registration
- capability-mediated access
- DMA descriptors
- CPU-local DMA staging
- future fabric-enforced memory windows

The long-term security model still points toward the Fabric acting as a hardware enforcement layer.

## What Changed From the Seed Design

The seed design staged the FPGA from optional smart glue into Ring 0 fabric over several revisions.

The current design moves the Fabric Northbridge concept forward as the central organizing model earlier in the architecture documents.

This does not reject the seed design. It refines it.

The project still preserves:

- staged bring-up
- observability
- discrete W65C816 learning path
- FPGA-mediated long-term security
- modular backplane goals
- secure expansion goals

The main change is that the documents now describe the long-term fabric-centered design more explicitly.

## Current Canonical Direction

As of this rationale document, the canonical ATARIX direction is:

```text
W65C816 CPU card
    -> CPU-local SRAM
    -> CPU-local DMA memory
    -> Fabric Northbridge interface
    -> modular backplane
    -> ECP5 Fabric Northbridge
    -> Service Directory
    -> fabric memory, storage, networking, accelerators, and future CPU cards
```

The supervisor management plane remains separate.

The firmware model is hybrid.

The toolchain model is cross-development first.

The software model is service-oriented.

The security model remains capability-mediated and distrusts optional hardware by default.

## Practical Next Step

The next important work is to convert rationale into implementation contracts:

- Discovery ROM binary format
- Service Directory binary format
- Boot Information Block
- Mailbox packet format
- Fabric register map
- Kernel image format
- CPU-local DMA descriptor format

After those contracts exist, the project should move to emulator and firmware bring-up rather than continuing to add high-level architecture prose.

<!-- AEMS-MIGRATED-SOURCE: docs/design.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design.md -->
### Integrated source: `docs/design.md`

# ATARIX Architecture Design Document

## Purpose

ATARIX is a modular cyberdeck-oriented experimental workstation platform built around the W65C816, a staged FPGA system fabric, a supervisor microcontroller, and a secure heterogeneous accelerator model.

The project is intended to support long-lived experimentation in operating systems, FPGA-based system architecture, capability-oriented security, heterogeneous computing, and retro-modern workstation design.

## Design Philosophy

The system is intentionally staged. Each revision must produce a useful, testable, and debuggable computer rather than a dependency on future hardware.

Guiding principles:

1. Observable hardware.
2. Incremental complexity.
3. Recovery before optimization.
4. Capability-based expansion.
5. Explicit trust boundaries.
6. Long service life.

## System Architecture

The long-term architecture consists of:

- W65C816 CPU subsystem
- Supervisor microcontroller
- FPGA-mediated system fabric
- Active backplane
- Device discovery framework
- Capability broker
- Accelerator framework

The architecture evolves from a simple single-board computer into a modular workstation platform.

## Revision Roadmap

### Rev A

Single W65C816 bring-up platform.

Features:

- W65C816
- ROM
- SRAM
- UART
- GPIO
- Discrete address decode
- Monitor firmware

### Rev B

Modularization phase.

Features:

- CPU card preparation
- Shared services
- W5500 networking
- Golden ROM recovery
- Active backplane planning

### Rev C

FPGA chipset phase.

Features:

- Address decoding
- Interrupt routing
- Mailboxes
- DMA foundations
- Device windows

### Rev D

Protected multiprocessing phase.

Features:

- Dual CPU operation
- MMU concepts
- DMA mediation
- Security rings
- Fault containment

### Rev E

Accelerator phase.

Features:

- ARM compute modules
- FPGA accelerators
- Capability broker
- Secure command queues
- Mediated DMA

## Security Model

Devices are not trusted by default.

Capabilities describe:

- Resource ownership
- Allowed operations
- Address windows
- DMA permissions
- Revocation rules

The security architecture becomes increasingly important as multiprocessing and accelerators are introduced.

## Supervisor Controller

Responsibilities include:

- Reset sequencing
- Power monitoring
- Clock management
- Recovery functions
- FPGA configuration support
- Fault logging
- Board identity management

## FPGA Fabric

Early FPGA responsibilities:

- Address decode
- Interrupt routing
- Mailboxes
- Timers

Later responsibilities:

- DMA mediation
- Bus arbitration
- Device discovery
- Accelerator attachment
- Capability enforcement assistance

## Software Implications

Early software:

- ROM monitor
- Memory test
- Serial loader
- Device diagnostics

Later software:

- Capability broker
- Device registry
- DMA manager
- Multiprocessor services
- Accelerator runtime

## Current Status

Architectural design and planning phase.

<!-- AEMS-MIGRATED-SOURCE: docs/discovery-rom-and-slot-architecture-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/discovery-rom-and-slot-architecture-v1.md -->
### Integrated source: `docs/discovery-rom-and-slot-architecture-v1.md`

# Discovery ROM and Slot Architecture v1

## Purpose

This document expands the card-discovery model used by ATARIX and records the current architectural direction discussed during design reviews.

The goal is that software binds to capabilities and services rather than physical slot numbers.

## Design Inspiration

Architectural influences include:

- Apple II slot discovery concepts
- NuBus declaration ROMs
- Open Firmware device descriptions
- PCI capability discovery
- Modern service-oriented fabrics

## Core Rule

Applications and operating systems should not depend on:

- Slot numbers
- Backplane position
- Fixed interrupt assignments
- Fixed memory windows

Instead they should bind to discovered services.

## Discovery ROM

Every intelligent card should expose a Discovery ROM.

Conceptually:

```text
Card Inserted
    -> Discovery ROM Read
    -> Capability Enumeration
    -> Resource Enumeration
    -> Service Registration
    -> Runtime Binding
```

## Discovery Information

Recommended fields:

- Vendor identifier
- Device identifier
- Card class
- Card revision
- Firmware revision
- Capability list
- Resource descriptors
- Service endpoints
- DMA requirements
- Interrupt requirements
- Diagnostic information

## Example Classes

Examples include:

- CPU card
- Memory card
- Storage card
- Network card
- Framebuffer card
- Accelerator card
- Supervisor card
- Development card

## Service Registration

A discovered card registers services with the Fabric Northbridge.

Example:

```text
Network Card
    -> Network Service
    -> DMA Capability
    -> Interrupt Endpoint
```

The operating system consumes the resulting service directory.

## Resource Descriptors

Resources should be described rather than hard-coded.

Examples:

- Mailbox endpoints
- DMA channels
- Interrupt vectors
- Memory windows
- MMIO regions
- Shared buffers
- Boot services

## Boot-Time Enumeration

Firmware should:

1. Enumerate every card.
2. Validate Discovery ROM structures.
3. Register capabilities.
4. Build the service directory.
5. Publish a Boot Information Block.
6. Transfer control to the loader or kernel.

## Hot-Plug Considerations

Hot-plug is not required for Rev A.

However, the discovery model should not prevent future support for:

- card insertion
- card removal
- service re-registration
- capability updates

## Supervisor Integration

The supervisor processor maintains an independent view of hardware health.

Examples:

- slot power status
- thermal alarms
- watchdog events
- fault history
- recovery operations

The supervisor may assist discovery and diagnostics but should not become the sole source of hardware truth.

## Long-Term Goal

The long-term goal is a self-describing workstation fabric in which hardware announces capabilities, firmware constructs a service directory, and operating systems bind to services rather than buses, addresses, or slots.

<!-- AEMS-MIGRATED-SOURCE: docs/discovery-rom-format.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/discovery-rom-format.md -->
### Integrated source: `docs/discovery-rom-format.md`

# ATARIX Discovery ROM Format

## Status

Draft specification for the ATARIX hybrid discovery format.

This design uses a fixed header followed by typed length-value records. It is intended to be simple enough for W65C816 monitor firmware to parse while remaining extensible enough for later CPU cards, FPGA services, backplane modules, and accelerators.

## Design Choice

ATARIX uses a hybrid discovery model:

- Fixed header for fast identification.
- Vendor, device, class, and revision fields for stable driver matching.
- TLV records for capabilities, memory windows, interrupts, DMA, mailboxes, human-readable names, and future extensions.

This is inspired by NuBus-style self-description, PCI-style identity fields, and device-tree-style extensibility, but it is deliberately simpler than any of those systems.

## Goals

1. Easy parsing from ROM monitor firmware.
2. Stable driver binding.
3. Extensible capability declaration.
4. Human-readable diagnostics.
5. Compatibility with future capability-based security.
6. No mandatory dynamic allocation during early boot.

## Byte Order

All multi-byte fields are little-endian.

## Alignment

Discovery records should be aligned to 2-byte boundaries.

Padding bytes must be zero.

## Fixed Header

```text
Offset  Size  Field
+00     4     Magic: "ATDX"
+04     1     Format Major Version
+05     1     Format Minor Version
+06     2     Header Length
+08     2     Total Length
+0A     2     Header Checksum
+0C     2     Vendor ID
+0E     2     Device ID
+10     2     Device Class
+12     2     Device Subclass
+14     2     Device Revision
+16     2     Flags
+18     4     TLV Offset
+1C     4     Reserved
```

Minimum header length:

```text
32 bytes
```

## Header Fields

### Magic

The magic field must contain:

```text
ATDX
```

Meaning:

```text
ATARIX Discovery
```

### Format Version

Major version changes indicate incompatible format changes.

Minor version changes indicate backward-compatible extensions.

### Header Length

Length of the fixed header in bytes.

### Total Length

Total discovery structure length in bytes, including header, TLV records, and padding.

### Header Checksum

A simple 16-bit checksum over the fixed header with the checksum field treated as zero.

This is not a security feature. It is only a corruption-detection feature.

### Vendor ID

Vendor or project identifier.

Reserved values:

```text
$0000 Invalid
$0001 ATARIX Project
$FFFF Experimental / Local
```

### Device ID

Vendor-defined device identifier.

### Device Class

Primary device class.

```text
$0000 Unknown
$0001 CPU Card
$0002 Memory Device
$0003 Serial Device
$0004 Network Device
$0005 Storage Device
$0006 FPGA Fabric Function
$0007 Supervisor Controller
$0008 Timer Device
$0009 DMA Engine
$000A Accelerator
$000B Backplane Service
$8000-$FFFF Experimental
```

### Device Subclass

Class-specific subtype.

### Device Revision

Hardware or firmware revision.

### Flags

```text
Bit 0 Boot Critical
Bit 1 Requires Driver
Bit 2 Has Interrupts
Bit 3 DMA Capable
Bit 4 Mailbox Capable
Bit 5 Capability Restricted
Bit 6 Hot Reset Safe
Bit 7 Supervisor Visible
Bits 8-15 Reserved
```

## TLV Record Format

Each TLV record begins with:

```text
Offset  Size  Field
+00     1     Type
+01     1     Length
+02     N     Value
```

Length is the size of the value field only.

A type of `$00` marks the end of records.

## Standard TLV Types

```text
$00 End
$01 Human-Readable Name
$02 Description
$03 Memory Window
$04 I/O Register Window
$05 Interrupt Source
$06 DMA Capability
$07 Mailbox Endpoint
$08 Firmware Version
$09 Driver Binding
$0A Capability Requirement
$0B Power Requirement
$0C Clock Requirement
$0D Reset Behavior
$0E Slot Information
$0F Diagnostic Entry Point
$10 Vendor-Specific Data
$80-$FF Experimental
```

## Name Record

Type:

```text
$01
```

Value:

```text
ASCII string, not necessarily null-terminated
```

Example:

```text
ATARIX Rev B Supervisor
```

## Memory Window Record

Type:

```text
$03
```

Value:

```text
Offset  Size  Field
+00     3     Base Address, 24-bit
+03     3     Length, 24-bit
+06     1     Attributes
```

Attributes:

```text
Bit 0 Readable
Bit 1 Writable
Bit 2 Executable
Bit 3 DMA Visible
Bit 4 Cacheable, future use
Bits 5-7 Reserved
```

## I/O Register Window Record

Type:

```text
$04
```

Value:

```text
Offset  Size  Field
+00     3     Base Address, 24-bit
+03     2     Length, 16-bit
+05     1     Access Width
```

Access width:

```text
$01 8-bit
$02 16-bit
$03 Mixed
```

## Interrupt Source Record

Type:

```text
$05
```

Value:

```text
Offset  Size  Field
+00     1     Interrupt Source ID
+01     1     Default Priority
+02     1     Flags
```

Flags:

```text
Bit 0 Level Triggered
Bit 1 Edge Triggered
Bit 2 Maskable
Bit 3 Wake Capable
```

## DMA Capability Record

Type:

```text
$06
```

Value:

```text
Offset  Size  Field
+00     1     DMA Channels
+01     1     Address Width
+02     1     Flags
```

Address width is measured in bits.

Flags:

```text
Bit 0 Read From Memory
Bit 1 Write To Memory
Bit 2 Scatter/Gather Capable
Bit 3 Requires Capability Grant
```

## Mailbox Endpoint Record

Type:

```text
$07
```

Value:

```text
Offset  Size  Field
+00     1     Endpoint ID
+01     1     Max Message Size Low
+02     1     Max Message Size High
+03     1     Flags
```

Flags:

```text
Bit 0 Interrupt On Receive
Bit 1 Polling Supported
Bit 2 Responses Supported
Bit 3 Capability Required
```

## Driver Binding Record

Type:

```text
$09
```

Value:

```text
ASCII string
```

Example:

```text
atarix,w5500
```

## Capability Requirement Record

Type:

```text
$0A
```

Value:

```text
Offset  Size  Field
+00     2     Required Capability Class
+02     2     Required Capability Flags
```

This record allows a device to declare that it requires mediated access before normal use.

## End Record

Type:

```text
$00
```

Length:

```text
$00
```

## Parser Requirements

A minimal parser must:

1. Verify magic.
2. Check major version.
3. Verify header length.
4. Verify total length is reasonable.
5. Optionally verify header checksum.
6. Iterate TLV records until end marker or total length.
7. Ignore unknown TLV types.

## Monitor Commands

Future ROM monitor commands should support:

```text
DISCOVER
DISCOVER LIST
DISCOVER SHOW <device>
DISCOVER RAW <device>
```

## Open Questions

- Final checksum algorithm.
- Whether total length should be 16-bit or 24-bit.
- Whether device identity should include serial number fields.
- Whether discovery records live in ROM, EEPROM, FPGA block RAM, or supervisor-provided memory.
- Whether human-readable strings should be ASCII-only or UTF-8.

## Versioning Rule

Version 1 discovery structures must remain parseable by future firmware.

Unknown TLV records must be ignored unless marked as mandatory by a future extension.

<!-- AEMS-MIGRATED-SOURCE: docs/fpga-fabric.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/fpga-fabric.md -->
### Integrated source: `docs/fpga-fabric.md`

# ATARIX FPGA Fabric Architecture

## Purpose

The FPGA fabric serves as the evolutionary bridge between a simple W65C816 computer and a modular workstation architecture.

The FPGA begins as programmable glue logic and gradually becomes a mediated system fabric.

## Initial Platform

Development target:

- Lattice ECP5
- ULX3S development board

Reasons:

- Open-source tooling
- Strong community support
- Adequate logic resources
- Long-term availability

## Rev C Responsibilities

### Address Decode

The FPGA replaces portions of discrete decode logic.

Responsibilities:

- Chip selects
- Address windows
- Device routing

### Interrupt Router

The FPGA centralizes interrupt management.

Capabilities:

- Interrupt prioritization
- Routing tables
- Masking
- Status reporting

### Mailboxes

Mailbox services support:

- CPU-to-device messaging
- CPU-to-CPU messaging
- Supervisor communication

### Timers

Fabric timers provide:

- System tick generation
- Benchmark timing
- Watchdog support

## DMA Engine

Future DMA support includes:

- Transfer descriptors
- Completion notification
- Access validation
- Fault reporting

DMA operations should eventually be capability mediated.

## Discovery Services

Each attached module should expose:

- Vendor ID
- Device ID
- Version information
- Capability records

The design is influenced by NuBus-style discovery concepts.

## Fabric Registers

Candidate blocks:

- System control
- Interrupt controller
- Mailboxes
- DMA control
- Discovery services
- Performance counters

## Future Responsibilities

- Bus arbitration
- Device isolation
- Capability enforcement assistance
- Accelerator attachment
- Fabric-level diagnostics

## Design Rule

The FPGA must remain documented.

Every register, mailbox, interrupt route, and DMA mechanism should be described by a stable specification.

<!-- AEMS-MIGRATED-SOURCE: docs/os-architecture.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/os-architecture.md -->
### Integrated source: `docs/os-architecture.md`

# ATARIX Operating System Architecture

## Goals

The operating system should evolve with the hardware rather than being designed in isolation.

The system begins as monitor firmware and grows into a workstation operating system.

## Boot Flow

### Stage 0

- Hardware reset
- Supervisor initialization
- ROM selection

### Stage 1

- ROM monitor startup
- Memory test
- Console initialization

### Stage 2

- Program loader
- Device discovery
- Service initialization

### Stage 3

- Kernel startup
- Driver registration
- Scheduler initialization

## Kernel Services

Candidate services:

- Memory management
- Interrupt management
- Mailboxes
- Device registry
- Timer services

## Capability Broker

Responsibilities:

- Resource grants
- DMA authorization
- Device permissions
- Revocation support

## Driver Model

Drivers should register through a discovery framework.

Each driver should describe:

- Device identity
- Capabilities
- Resource requirements

## Multiprocessor Services

Future support includes:

- Message passing
- Shared-memory coordination
- Synchronization primitives

## Accelerator Runtime

Future support includes:

- Accelerator discovery
- Command queues
- Memory grants
- Completion notification

## Long-Term Goal

A modular workstation operating system capable of supporting secure experimentation across CPUs, FPGA services, and heterogeneous accelerators.

<!-- AEMS-MIGRATED-SOURCE: docs/rev-a-hardware-baseline.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/rev-a-hardware-baseline.md -->
### Integrated source: `docs/rev-a-hardware-baseline.md`

# ATARIX Rev A Hardware Baseline

## Status

Draft baseline hardware-selection record.

This document records concrete Rev A hardware choices that should guide the CPU card, supervisor, fabric-controller, and backplane specifications.

## Fabric Controller

Selected development platform:

```text
ULX3S with Lattice LFE5U-85F ECP5 FPGA
```

Rationale:

- Largest commonly available ULX3S ECP5 configuration.
- Adequate headroom for fabric-controller experiments.
- Open-source FPGA toolchain support.
- Suitable for mailbox, interrupt, DMA, discovery, debug, and health-counter logic.
- Practical bridge between prototype and custom fabric-controller card.

The ULX3S LFE5U-85F should be treated as the Rev A/B fabric-controller target unless superseded by a custom card.

## Fabric-Controller Local Memory

The ULX3S / ECP5 fabric controller should use its available SDRAM as fabric infrastructure memory.

Baseline:

```text
64 MB SDRAM on the ULX3S / ECP5 fabric-controller side
```

Primary uses:

- Mailbox storage.
- DMA queues.
- Discovery cache.
- Capability cache.
- Object-window cache.
- Trace buffers.
- Netboot staging.
- Fault logs and diagnostic buffers.

Design rule:

```text
Fabric memory is infrastructure memory.

Memory-service memory is system memory.

CPU-local memory is execution memory.
```

Rev A should not require the fabric controller to carry the burden of large system RAM. Large memory belongs on future memory-service cards.

## Supervisor MCU

Selected supervisor MCU family:

```text
RP2350
```

Responsibilities:

- RTC coordination.
- Watchdog supervision.
- Fault logging.
- Reset sequencing.
- Power-good monitoring.
- Recovery-mode control.
- Management-plane communication.
- DIP-switch and recovery-input handling.
- I2C management-bus coordination.
- Per-slot identity and health inventory.

The RP2350 should be treated as the primary supervisor-controller target.

## Per-Card Management and I2C Baseline

Rev A cards should expose a minimum independent management cluster reachable by the supervisor and/or fabric-controller management path.

Required per-card management objects:

```text
Identity EEPROM / FRU EEPROM
Temperature Sensor
Voltage Monitor
Current Monitor
Status / Control Register
```

Recommended per-card management objects:

```text
Fan Tachometer
Fan Controller
Additional ADC Channels
Environmental Sensors
```

Optional / high-reliability objects:

```text
Secure Element
Calibration EEPROM
Management MCU
```

These objects should function independently from the card's main processing logic whenever practical.

See:

```text
docs/card-identity-eeprom.md
```

## CPU-Card Memory Target

The W65C816 CPU card should provide the maximum native RAM the CPU can directly address, where practical.

Target CPU-local RAM:

```text
16 MiB CPU-local addressable RAM target
```

This corresponds to the W65C816 native 24-bit physical address space.

The CPU card should still preserve a reliable boot and recovery layout for:

- Direct page.
- Stack.
- ROM monitor workspace.
- Diagnostics.
- Netboot staging.
- Recovery operation.

Memory planning principle:

```text
Populate the CPU card with the maximum practical W65C816-addressable local RAM while preserving deterministic boot, recovery, and observability.
```

Large SDRAM or future DDR memory should be introduced through the fabric-controller path or through a controlled memory-service window, not as a reason to eliminate CPU-local execution RAM.

## Network Service Baseline

Rev A must include a network service path sufficient for netboot, NTP, and remote diagnostics.

Acceptable Rev A implementation:

```text
W5500 Ethernet on the CPU card
```

Alternative Rev A implementation:

```text
Dedicated network service card available at boot
```

Requirement:

```text
The base Rev A system must not depend on a missing optional network card in order to perform netboot.
```

Networking may move to a service card in later revisions, but Rev A must define a concrete network service path.

See:

```text
docs/netboot-ntp-v1.md
```

## Flash Device

Flash device selection remains open.

Candidate roles:

```text
CPU boot ROM / flash
Golden ROM image
FPGA bitstream storage
Supervisor firmware storage
Netboot fallback image
Configuration storage
```

Selection criteria:

- Availability.
- Programming simplicity.
- In-circuit update support.
- Recovery safety.
- Compatibility with supervisor-controlled golden image selection.
- Electrical compatibility with selected CPU, FPGA, and supervisor domains.

## CPU Clock Target

Rev A CPU card should be designed to run the W65C816 as fast as practical while preserving a deterministic debug path.

Required clock modes:

```text
1 MHz Debug
4 MHz Bring-up
8 MHz Conservative Operation
14 MHz Target Operation
Fastest Stable Experimental Mode
Single-step or gated-clock debug mode
```

The goal is not to guarantee maximum frequency on the first board.

The goal is to design the card so frequency limits are set by measurable timing margins rather than by poor observability, weak clocking, or avoidable signal-integrity problems.

## Design Implications

These selections imply:

- The fabric-controller specification should assume the LFE5U-85F resource class.
- The supervisor specification should assume RP2350 GPIO, timer, and management capabilities.
- The backplane should reserve enough signals for ULX3S-based fabric experiments.
- The CPU card should include strong diagnostic visibility around clock, RDY, chip select, interrupt, and wait-state behavior.
- The CPU card should target the full W65C816 native 16 MiB addressable RAM footprint where practical.
- SDRAM should be treated as a fabric-visible infrastructure resource with explicit ownership and test strategy.
- Flash selection should be tied to recovery architecture, not selected only by capacity.
- A Rev A network service path is mandatory for netboot and NTP.
- Per-card I2C management objects should be reachable independently of the card data plane.

## Related Documents

```text
docs/ulx3s-backplane-controller.md
docs/supervisor-card-v1.md
docs/cpu-card-architecture-v1.md
docs/clock-reset-spec.md
docs/netboot-ntp-v1.md
docs/diagnostic-access-v1.md
docs/card-identity-eeprom.md
```

## Open Questions

- Exact ULX3S connector strategy.
- Exact SDRAM topology and maximum practical capacity.
- Exact 16 MiB CPU-local RAM implementation strategy.
- Whether large memory is attached through ULX3S, CPU card, or a memory service card.
- Flash part family and package.
- Whether boot flash and FPGA bitstream flash are separate.
- CPU-card oscillator strategy.
- Maximum verified W65C816 frequency target after timing tests.
- Whether the first Rev A board should include selectable voltage-domain measurement jumpers.
- Whether W5500 is placed on the CPU card or implemented as a boot-critical network service card.

## Design Principle

Rev A should be ambitious in capability but conservative in recoverability.

The machine should be able to run fast, but it must always be possible to slow it down, observe it, recover it, and explain what failed.

<!-- AEMS-MIGRATED-SOURCE: docs/specification-roadmap.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/specification-roadmap.md -->
### Integrated source: `docs/specification-roadmap.md`

# ATARIX Specification Roadmap

## Purpose

This document tracks the remaining specifications required to move ATARIX from architecture definition into implementation.

Guiding principle:

```text
Specification
    -> Header
    -> Implementation
    -> Test
```

Every specification should eventually produce:

- a public header under include/atarix/
- firmware and/or emulator code
- validation tests

---

# Architecture Synchronization

- [ ] architecture-timeline.md
- [ ] ADR-CPU-WIDTH-IS-NOT-FABRIC-WIDTH.md
- [ ] README normalization pass
- [ ] Archive synchronization pass

---

# Identity Layer

## Identity EEPROM v1

Status: Planned

Outputs:

- docs/identity-eeprom-v1.md
- include/atarix/identity_eeprom.h

Defines:

- board identity
- manufacturing identity
- serial numbering
- capability flags
- revision tracking

## Discovery ROM Format v1

Status: Planned

Outputs:

- docs/discovery-rom-format-v1.md
- include/atarix/discovery.h

Defines:

- device discovery
- service advertisement
- resource description

## Capability Record Format v1

Status: Planned

Outputs:

- docs/capability-record-format-v1.md
- include/atarix/capability.h

Defines:

- service capabilities
- permissions
- feature advertisement

---

# Discovery Layer

## Service Directory Format v1

Status: Planned

Outputs:

- docs/service-directory-format-v1.md
- include/atarix/service.h

Defines:

- discovered services
- service handles
- provider records

## Boot Information Block v1

Status: Planned

Outputs:

- docs/boot-information-block-v1.md
- include/atarix/bib.h

Defines:

- firmware-to-kernel handoff
- service directory location
- memory information
- boot information

---

# Transport Layer

## Mailbox Protocol v1

Status: Planned

Outputs:

- docs/mailbox-protocol-v1.md
- include/atarix/mailbox.h

Defines:

- request/reply transport
- transaction IDs
- DMA requests
- discovery traffic

## DMA Descriptor Format v1

Status: Planned

Outputs:

- docs/dma-descriptor-format-v1.md
- include/atarix/dma.h

Defines:

- DMA descriptors
- DMA handles
- completion records

## Fabric CSR Map v1

Status: Planned

Outputs:

- docs/fabric-csr-map-v1.md

Defines:

- software-visible fabric registers
- mailbox CSRs
- interrupt CSRs
- DMA CSRs

---

# Operations Layer

- [ ] backplane-bios-v1.md
- [ ] atarix-monitor-v1.md
- [ ] instrumentation-service-v1.md
- [ ] directory-service-v1.md
- [ ] factory-provisioning-v1.md
- [ ] cpu-validation-strategy-v1.md

---

# Source Tree Milestones

## Firmware

```text
src/firmware/monitor/
```

Goal:

```text
RESET
 -> UART
 -> Identity EEPROM
 -> Monitor Prompt
```

## Emulator

```text
src/emulator/fabric/
src/emulator/discovery/
src/emulator/mailbox/
```

Goal:

Architecturally accurate models of:

- discovery
- service enumeration
- mailbox transport
- Boot Information Block creation

---

# First Boot Milestone

Success criteria:

```text
Power On
 ↓
Supervisor
 ↓
Fabric
 ↓
CPU ROM
 ↓
Identity EEPROM
 ↓
Discovery ROM
 ↓
Service Directory
 ↓
Boot Information Block
 ↓
Kernel Stub
```

Expected monitor output:

```text
ATARIX ROM MONITOR v0.1

Board: Rev A CPU Card
Vendor: ATARIX
Serial: 000001

>
```

When this milestone is reached, ATARIX transitions from architecture project to functioning platform.
