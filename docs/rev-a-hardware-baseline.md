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

The RP2350 should be treated as the primary supervisor-controller target.

## Memory Target

Rev A should support the maximum practical SDRAM configuration available through the chosen ULX3S and board-level design constraints.

Memory planning principle:

```text
Maximize available RAM without compromising bring-up reliability.
```

The CPU card should still include enough local SRAM or reliable boot RAM for:

- Direct page.
- Stack.
- ROM monitor workspace.
- Diagnostics.
- Netboot staging.
- Recovery operation.

Large SDRAM should be introduced through the fabric-controller path or through a controlled memory window, not as a reason to eliminate local CPU-card bring-up RAM.

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
- SDRAM should be treated as a fabric-visible resource with explicit ownership and test strategy.
- Flash selection should be tied to recovery architecture, not selected only by capacity.

## Related Documents

```text
docs/ulx3s-backplane-controller.md
docs/supervisor-card-v1.md
docs/cpu-card-architecture-v1.md
docs/clock-reset-spec.md
docs/netboot-ntp-v1.md
docs/diagnostic-access-v1.md
```

## Open Questions

- Exact ULX3S connector strategy.
- Exact SDRAM topology and maximum practical capacity.
- Whether large memory is attached through ULX3S, CPU card, or a memory service card.
- Flash part family and package.
- Whether boot flash and FPGA bitstream flash are separate.
- CPU-card oscillator strategy.
- Maximum verified W65C816 frequency target after timing tests.
- Whether the first Rev A board should include selectable voltage-domain measurement jumpers.

## Design Principle

Rev A should be ambitious in capability but conservative in recoverability.

The machine should be able to run fast, but it must always be possible to slow it down, observe it, recover it, and explain what failed.