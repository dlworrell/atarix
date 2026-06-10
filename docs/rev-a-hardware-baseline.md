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