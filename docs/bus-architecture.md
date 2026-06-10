# ATARIX Bus Architecture

## Status

Draft architectural decision record.

This document records the selected direction for the ATARIX card and backplane architecture.

## Design Decision

ATARIX will use a hybrid modular bus architecture inspired by Sun workstation expansion buses and Apple NuBus-style self-description.

The backplane should not simply expose the raw W65C816 bus to every slot.

Instead, the W65C816 bus remains local to the CPU card where possible, while the backplane exposes a structured fabric-oriented interface for devices, services, accelerators, and future CPU modules.

## Influences

### Sun SPARCstation-Style Expansion

ATARIX should take inspiration from Sun workstation expansion systems such as SPARCstation-class modular I/O.

Relevant ideas:

- Workstation-oriented modular expansion.
- Stable card/backplane mechanical model.
- Clean separation between host processor and expansion devices.
- Device slots as first-class architectural participants.

### NuBus-Style Self Description

ATARIX should also take inspiration from NuBus-style cards.

Relevant ideas:

- Cards identify themselves.
- Cards describe resources.
- Driver matching can rely on structured metadata.
- Expansion devices are more than simple fixed-address peripherals.

ATARIX discovery is defined separately in:

```text
docs/discovery-rom-format.md
```

## Hybrid ATARIX Approach

The selected model is:

```text
W65C816 local bus
        |
CPU card local glue / bridge
        |
ATARIX fabric interface
        |
Active backplane
        |
Service cards / fabric cards / accelerator cards
```

The CPU card may expose selected bus-derived transactions, but the backplane should prefer a controlled fabric interface rather than every raw CPU signal.

## Why Not a Raw 65C816 Backplane?

A raw CPU bus backplane is simple, but it creates long-term problems:

- Poor scaling to multiple CPU cards.
- Difficult signal integrity.
- Weak fault isolation.
- Difficult DMA mediation.
- No natural discovery model.
- No clean authority boundary.
- Harder accelerator integration.

ATARIX is intended to become more than a single-board 65C816 machine, so the bus should not be designed around the simplest possible first revision only.

## Why Not a Fully Abstract Fabric Only?

A fully abstract fabric would lose the value of the W65C816 architecture and make early bring-up harder.

ATARIX should preserve:

- Observable CPU behavior.
- Simple Rev A diagnostics.
- ROM monitor access.
- Direct hardware understanding.

Therefore, the CPU-local bus remains important, especially in Rev A and Rev B.

## Layered Bus Model

### Layer 0: CPU Local Bus

Scope:

- CPU card only.

Contains:

- W65C816 address bus.
- W65C816 data bus.
- CPU control signals.
- Local ROM or boot mapping.
- Local SRAM if used.
- Local glue logic.

### Layer 1: CPU Bridge Interface

Scope:

- Boundary between CPU card and system fabric.

Responsibilities:

- Translate CPU bus cycles into fabric transactions.
- Isolate CPU card timing from backplane timing.
- Provide wait-state or ready signaling.
- Support debug visibility.

### Layer 2: ATARIX Fabric Interface

Scope:

- Backplane-level logical interface.

Responsibilities:

- Addressed transactions.
- Mailbox delivery.
- Interrupt routing.
- Discovery access.
- DMA request mediation.
- Capability-aware access control.

### Layer 3: Active Backplane

Scope:

- Slot-to-slot system interconnect.

Responsibilities:

- Slot identification.
- Reset distribution.
- Clock distribution.
- Power distribution.
- Fabric routing.
- Fault containment.

### Layer 4: Service and Accelerator Modules

Scope:

- Cards attached to the backplane.

Examples:

- Network card.
- Storage card.
- FPGA service card.
- ARM compute module.
- Future accelerator.
- Future secondary CPU card.

## Card Identity

Every nontrivial card should provide discovery information.

Required identity fields:

- Vendor ID.
- Device ID.
- Device class.
- Revision.
- Capability records.
- Optional driver binding string.

## Transaction Model

The fabric interface should support these transaction classes:

```text
Register Read
Register Write
Mailbox Send
Mailbox Receive
Discovery Read
DMA Request
Interrupt Signal
Fault Report
```

Early revisions may implement only a subset.

## CPU Card Role

The primary CPU card is not just a passive bus master.

It is a participant in a larger system fabric.

Responsibilities:

- Boot monitor execution.
- Service discovery.
- Fabric register access.
- Mailbox participation.
- Capability broker participation in later revisions.

## FPGA Role

The FPGA is the natural fabric bridge and enforcement assistant.

Responsibilities may include:

- Address decode.
- Transaction routing.
- Interrupt routing.
- Mailbox handling.
- Discovery table exposure.
- DMA mediation.
- Fault logging.

The FPGA should not be treated as an undocumented black box.

## Backplane Design Implications

The backplane should include signal groups for:

- Power.
- Ground.
- Reference clock.
- Reset.
- Slot identification.
- Interrupt or event signaling.
- Fabric data path.
- Fabric control path.
- Discovery access.
- Debug and recovery.

The exact pinout is deferred to:

```text
docs/backplane-pinout-v1.md
```

## CPU Card Design Implications

The CPU card should include:

- Local W65C816 bus.
- Local bridge or glue logic.
- Debug access.
- Reset control.
- Clock input or local clock selection.
- Discovery record.
- Fabric-facing interface.

The exact pinout is deferred to:

```text
docs/cpu-card-pinout-v1.md
```

## Security Implications

Because the backplane is not a raw shared CPU bus, ATARIX can enforce stronger rules:

- Devices are not automatically trusted.
- DMA requests can be mediated.
- Discovery can declare required capabilities.
- Faults can be isolated per slot.
- Accelerators can receive explicit memory grants.

See:

```text
docs/capability-model.md
docs/dma-engine-v1.md
```

## Development Phasing

### Rev A

Raw W65C816 system for bring-up.

### Rev B

CPU card preparation and early bridge concepts.

### Rev C

FPGA-mediated fabric interface begins.

### Rev D

Multi-CPU and mediated resource experiments.

### Rev E

Accelerator and heterogeneous compute framework.

## Design Rule

Keep the W65C816 bus local where possible.

Expose a structured, discoverable, fabric-oriented interface to the backplane.

This preserves early debuggability while allowing ATARIX to grow into a modular workstation architecture.