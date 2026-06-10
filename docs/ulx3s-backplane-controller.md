# ULX3S Backplane Fabric Controller

## Status

Draft hardware design note.

This document records the decision that the first ATARIX backplane fabric controller should be implemented using a ULX3S-class Lattice ECP5 FPGA board mounted on, or connected directly to, the ATARIX backplane.

## Purpose

The ULX3S board provides a practical early FPGA platform for implementing and testing ATARIX fabric concepts before designing a custom fabric-controller card.

The ULX3S should be treated as the Rev A/B fabric-controller development platform rather than a temporary afterthought.

## Design Decision

The ATARIX backplane should include support for a ULX3S FPGA module with DIP-switch configuration.

The ULX3S fabric controller may provide:

- Address decode experiments.
- Interrupt routing.
- Mailbox services.
- Discovery-service exposure.
- DMA-controller experiments.
- Slot identification.
- Reset and clock coordination assistance.
- Debug and diagnostic logic.

## Backplane Role

The ULX3S-connected FPGA acts as the first implementation of the ATARIX fabric controller.

Conceptual topology:

```text
DIN41612 Backplane
        |
ULX3S / ECP5 Fabric Controller
        |
Fabric Control Logic
        |
CPU Cards / Service Cards / Accelerator Cards
```

## DIP-Switch Purpose

The backplane should expose a DIP-switch block associated with the fabric controller.

Candidate DIP-switch functions:

```text
Switch 0 Recovery Mode
Switch 1 Golden Bitstream / Normal Bitstream
Switch 2 Debug Clock Enable
Switch 3 Fabric Services Enable
Switch 4 DMA Enable
Switch 5 Discovery Override
Switch 6 Slot Test Mode
Switch 7 Reserved
```

The exact switch mapping may change, but the design should reserve manual configuration inputs from the beginning.

## Recovery Requirement

The system must support a human-controlled recovery path.

Recovery mode should be able to:

- Hold CPU cards in reset.
- Disable DMA.
- Disable accelerators.
- Select a known-good FPGA bitstream.
- Permit supervisor or UART diagnostics.
- Bring up the fabric in a minimal safe state.

## FPGA Development Advantages

Using ULX3S early provides:

- Lattice ECP5 toolchain compatibility.
- Open-source FPGA flow support.
- Existing board availability.
- Faster HDL experimentation.
- Reduced risk before custom PCB design.

## Integration With Existing Architecture

This design note connects to:

```text
docs/fpga-fabric.md
docs/bus-architecture.md
docs/backplane-spec.md
docs/clock-reset-spec.md
docs/interrupt-architecture-v1.md
docs/mailbox-protocol-v1.md
docs/dma-engine-v1.md
```

## Design Rule

The ULX3S fabric controller should be part of the backplane design from the beginning.

Do not design the backplane first and then try to attach the FPGA later.

The FPGA, DIP switches, recovery path, and slot-control signals are first-class backplane requirements.

## Open Questions

- Whether the ULX3S mounts directly to the backplane or through an adapter card.
- Which ULX3S connectors are used for fabric signals.
- Whether the DIP switch is on the backplane, adapter, or ULX3S carrier.
- How many FPGA I/O lines are reserved for slot control.
- Whether the supervisor MCU or ULX3S owns reset sequencing in Rev B.
- Whether FPGA configuration storage lives on the ULX3S or the backplane.