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
