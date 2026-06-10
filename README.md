# ATARIX

ATARIX is a modular experimental workstation architecture built around W65C816 CPU cards, an ECP5 FPGA fabric controller, a supervisor management plane, capability-mediated resource access, and a Sun/NuBus/UPA-inspired modular backplane.

The project is intended to support operating-system research, FPGA-based system architecture, secure device mediation, hardware bring-up, heterogeneous compute experiments, and retro-modern workstation design.

## Current Architectural Direction

ATARIX is not a raw W65C816 bus stretched across a backplane.

The current design direction is:

```text
W65C816 CPU Card
    -> CPU-local bus, SRAM, ROM, shim/buffer logic
    -> ATARIX fabric bridge
    -> DIN41612-style modular backplane
    -> ULX3S LFE5U-85F fabric controller
    -> service cards, memory services, accelerators, and future CPU cards
```

The supervisor management plane is separate from the main control/data/event fabric and remains responsible for reset, recovery, RTC, watchdogs, health monitoring, and fault logging.

## Rev A Baseline

Current Rev A baseline selections:

- **Fabric controller:** ULX3S with Lattice LFE5U-85F ECP5 FPGA
- **Supervisor MCU:** RP2350
- **CPU:** WDC W65C816S
- **Backplane direction:** DIN41612-style modular workstation backplane
- **CPU-card memory:** local SRAM/ROM for deterministic bring-up
- **Large memory direction:** fabric-visible memory service rather than flat native 65C816 RAM
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

### Hardware Architecture

- [Rev A Hardware Baseline](docs/rev-a-hardware-baseline.md)
- [CPU Card Architecture v1](docs/cpu-card-architecture-v1.md)
- [Supervisor Card Architecture v1](docs/supervisor-card-v1.md)
- [Bus Architecture](docs/bus-architecture.md)
- [Backplane Specification](docs/backplane-spec.md)
- [CPU Card Specification](docs/cpu-card-spec.md)
- [ULX3S Backplane Fabric Controller](docs/ulx3s-backplane-controller.md)
- [Clock and Reset Specification](docs/clock-reset-spec.md)
- [Diagnostic Access v1](docs/diagnostic-access-v1.md)

### Addressing, Memory, and Registers

- [Memory Map](docs/memory-map.md)
- [Address Space Architecture](docs/address-space-architecture.md)
- [Register Map v1](docs/register-map-v1.md)

### Fabric, Communication, and Devices

- [FPGA Fabric Architecture](docs/fpga-fabric.md)
- [Mailbox Protocol v1](docs/mailbox-protocol-v1.md)
- [Interrupt Architecture v1](docs/interrupt-architecture-v1.md)
- [DMA Engine v1](docs/dma-engine-v1.md)
- [Discovery ROM Format](docs/discovery-rom-format.md)
- [Netboot and NTP Boot Services v1](docs/netboot-ntp-v1.md)

### Security, Recovery, and Management

- [Security Architecture](docs/security.md)
- [Capability Model](docs/capability-model.md)
- [Management Anomaly Detection v1](docs/management-anomaly-detection-v1.md)

### Tooling and Reference Designs

- [Toolchain Strategy](docs/toolchain.md)
- [Operating System Architecture](docs/os-architecture.md)
- [Vega816 Hardware Migration Analysis](docs/vega816-analysis.md)

## Architectural Influences

ATARIX draws inspiration from:

- W65C816 homebrew and workstation-class experimentation
- Vega816 CPU shim, buffering, DMA, and vector-pull-rewrite concepts
- BB816 backplane and interconnect experiments
- Apple NuBus-style self-description
- Sun SBus/UPA-style workstation fabric thinking
- Modern capability-based security and service-processor recovery models
- Lattice ECP5 / ULX3S FPGA development

## Core Principles

1. Keep the W65C816 local bus local to the CPU card.
2. Use a structured fabric interface instead of a raw shared CPU bus.
3. Make every major subsystem observable and diagnosable.
4. Treat devices as untrusted until granted explicit capabilities.
5. Use the supervisor management plane for reset, recovery, RTC, watchdogs, and fault logging.
6. Prefer staged bring-up over premature complexity.
7. Support large memory through managed memory services, not by pretending the 65C816 has a flat server-class address space.

## Near-Term Work

The next high-value documents and engineering tasks are:

1. `docs/fabric-controller-architecture-v1.md`
2. `docs/backplane-pinout-v1.md`
3. `docs/vector-pull-rewrite-v1.md`
4. KiCad project structure for:
   - `hardware/atarix-backplane/`
   - `hardware/atarix-cpu-card/`
   - `hardware/atarix-supervisor/`
   - `hardware/atarix-fabric-controller/`

## Status

Architectural design and early hardware-definition phase.

The project is transitioning from broad system architecture into fabric-controller definition, pin allocation, and schematic-capture planning.