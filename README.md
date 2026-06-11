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

## Documentation Index

### System Overview

- [System Specification v1](docs/system-spec-v1.md)
- [Architecture Design Document](docs/design.md)
- [Hardware Architecture](docs/hardware.md)
- [Software Architecture](docs/software.md)
- [Project Roadmap](docs/roadmap.md)
- [Development Philosophy](docs/development-philosophy.md)
- [Architecture Decision Records](docs/adr/README.md)

### Hardware Architecture

- [Rev A Hardware Baseline](docs/rev-a-hardware-baseline.md)
- [CPU Card Architecture v1](docs/cpu-card-architecture-v1.md)
- [Supervisor Card Architecture v1](docs/supervisor-card-v1.md)
- [Fabric Northbridge Architecture v1](docs/fabric-controller-architecture-v1.md)
- [Boot Firmware and BIOS Architecture v1](docs/boot-firmware-architecture-v1.md)
- [NuBus and UPA Architectural Influences](docs/nubus-upa-influences-v1.md)
- [Fabric Service Model v1](docs/fabric-service-model-v1.md)

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

Architectural design and early hardware-definition phase.

The project is transitioning toward Fabric Northbridge definition, service architecture refinement, memory-card architecture, and schematic-capture planning.