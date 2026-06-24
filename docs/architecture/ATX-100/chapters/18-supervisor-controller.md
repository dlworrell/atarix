# Chapter 18: Supervisor Controller

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/clock-reset-spec.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/clock-reset-spec.md -->
### Integrated source: `docs/clock-reset-spec.md`

# ATARIX Clock and Reset Specification

## Status

Draft hardware specification for clock generation, clock distribution, reset sequencing, supervisor control, and FPGA startup.

## Purpose

Clocking and reset behavior form the foundation of the ATARIX platform.

This specification establishes deterministic startup behavior for:

- W65C816 CPU cards
- Supervisor controller
- FPGA fabric
- Service modules
- Future accelerator cards

## Design Goals

1. Deterministic power-on behavior.
2. Recoverable startup sequence.
3. Debug-friendly operation.
4. Supervisor-controlled reset.
5. FPGA configuration before CPU release when required.
6. Future multi-CPU support.

## Clock Domains

Initial clock domains:

```text
Supervisor Clock Domain
CPU Clock Domain
FPGA Fabric Clock Domain
Peripheral Clock Domain
```

Future revisions may introduce accelerator-specific clock domains.

## Primary Oscillator

Recommended system reference:

```text
25 MHz
```

Reasons:

- Common FPGA reference frequency.
- Suitable for Ethernet timing derivation.
- Easy PLL generation.
- Good long-term availability.

## Supervisor Clock

The supervisor controller should start immediately after power stabilization.

Supervisor clock requirements:

```text
Independent of W65C816 startup
Independent of FPGA configuration status
Available during recovery mode
```

The supervisor must remain operational even if the CPU or FPGA fails.

## CPU Clock Generation

Initial supported CPU clocks:

```text
1 MHz Debug
4 MHz Bring-up
8 MHz Normal
14 MHz Experimental
```

Clock selection may be controlled by:

- Supervisor firmware
- DIP switches
- Configuration EEPROM
- Recovery mode settings

## Debug Clock Mode

Debug mode should support:

```text
Single Step
Slow Clock
Continuous Clock
```

The supervisor may optionally gate CPU clock generation during debugging.

## FPGA Clocking

FPGA clock sources:

```text
Reference Oscillator
PLL-Derived Clocks
Optional External Debug Clock
```

Fabric services should not assume CPU and FPGA clocks are identical.

Clock-domain crossing must be explicit.

## Reset Sources

Possible reset sources:

```text
Power-On Reset
Supervisor Reset
Manual Reset Button
Recovery Button
Watchdog Timeout
Software Reset Request
Fabric Fault Reset
```

## Reset Tree

Logical hierarchy:

```text
Power Good
    |
Supervisor Controller
    |
    +-- FPGA Reset
    |
    +-- CPU Reset
    |
    +-- Slot Reset
```

The supervisor is the root reset authority.

## Reset Sequencing

### Phase 1

Power rails stabilize.

### Phase 2

Supervisor exits reset.

### Phase 3

Supervisor validates power-good status.

### Phase 4

FPGA configured if required.

### Phase 5

Fabric services initialized.

### Phase 6

CPU released from reset.

### Phase 7

Boot monitor executes.

## Recovery Mode

Recovery mode may hold:

```text
CPU in Reset
DMA Disabled
Accelerators Disabled
```

while leaving:

```text
Supervisor Active
UART Active
FPGA Configuration Access Active
```

## Watchdog Architecture

Supervisor watchdog responsibilities:

- Detect stalled CPU.
- Detect failed boot sequence.
- Trigger recovery.
- Record reset cause.

Watchdog expiration should never immediately erase diagnostic information.

## Reset Cause Register

Recommended retained causes:

```text
Power-On Reset
Manual Reset
Recovery Request
Watchdog Reset
Software Reset
Fabric Fault
Unknown
```

The ROM monitor should display reset cause information during startup.

## Multi-CPU Considerations

Future systems may support:

```text
Global Reset
Per-CPU Reset
Per-Slot Reset
```

The supervisor should be capable of resetting a single CPU card without affecting the remainder of the system.

## Design Rules

1. The supervisor always starts first.
2. CPU release is intentional, never automatic.
3. DMA remains disabled during early boot.
4. Accelerators remain disabled during early boot.
5. Recovery mode must remain available after failed firmware updates.
6. Clock generation must remain observable during debugging.

## Open Questions

- Final oscillator frequency.
- Supervisor MCU selection.
- External clock input support.
- FPGA configuration storage location.
- Multi-CPU synchronization strategy.
- Watchdog timeout values.
- Whether clock gating occurs in supervisor hardware or FPGA fabric.

<!-- AEMS-MIGRATED-SOURCE: docs/memory-map.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/memory-map.md -->
### Integrated source: `docs/memory-map.md`

# ATARIX Memory Map

## Purpose

This document captures the first-pass ATARIX memory-map plan. It is intentionally conservative for Rev A and leaves room for FPGA-mediated banking, device windows, DMA control, and capability enforcement in later revisions.

## Design Rules

1. Rev A must be simple enough to debug with ordinary tools.
2. ROM, RAM, and UART must be easy to verify independently.
3. Later FPGA windows must not invalidate the Rev A bring-up model.
4. Device registers must be explicitly documented.
5. DMA-visible memory must eventually be mediated by the fabric.

## W65C816 Addressing Context

The W65C816 presents a 24-bit address space organized as 256 banks of 64 KiB each.

For early bring-up, ATARIX should treat bank `$00` as the most important bank because it contains reset vectors, interrupt vectors, direct-page assumptions, stack behavior, and most monitor activity.

## Rev A Draft Map

```text
Bank $00

$000000-$0000FF  Direct page / zero-page-compatible region
$000100-$0001FF  Hardware stack default region
$000200-$007FFF  Low RAM / monitor work area
$008000-$00BFFF  Expansion or device window candidate
$00C000-$00DFFF  I/O window candidate
$00E000-$00FFFF  ROM window, vectors at top of bank
```

This map favors easy decoding and easy monitor development over density.

## Rev A Required Regions

### RAM

Minimum useful RAM:

- Direct page
- Stack page
- Monitor variables
- Upload buffer
- Test area

### ROM

ROM must contain:

- Reset vector
- Native interrupt vectors
- Emulation-mode vectors if used during bring-up
- Monitor firmware
- Memory-test entry points
- Serial loader entry points

### UART

The UART should be placed in the I/O window and decoded simply.

Candidate location:

```text
$00C000-$00C00F  UART register block
```

### GPIO

Candidate location:

```text
$00C100-$00C1FF  GPIO register block
```

## Rev B Extensions

Rev B may add:

- W5500 Ethernet register window
- Supervisor mailbox
- Board identity registers
- Recovery control registers

Candidate locations:

```text
$00C200-$00C2FF  Supervisor mailbox
$00C300-$00C3FF  Board identity / configuration
$00C400-$00C7FF  W5500 window
```

## Rev C FPGA Register Space

Rev C introduces FPGA-mediated system registers.

Candidate locations:

```text
$00D000-$00D0FF  FPGA system control
$00D100-$00D1FF  Interrupt controller
$00D200-$00D2FF  Mailbox registers
$00D300-$00D3FF  DMA control
$00D400-$00D4FF  Timer block
$00D500-$00D5FF  Device discovery block
```

## Future Banked Memory

Banks above `$00` may eventually be used for expanded RAM, memory windows, accelerator buffers, or DMA-visible regions.

Candidate organization:

```text
$010000-$7FFFFF  General RAM / banked system memory
$800000-$BFFFFF  Device and accelerator windows
$C00000-$EFFFFF  Fabric-controlled mappings
$F00000-$FFFFFF  Reserved / ROM shadow / debug / recovery
```

This is provisional and should not be treated as a final hardware contract.

## DMA Visibility

DMA-capable devices must not receive unrestricted access to the full address space.

Future DMA regions should include:

- Owner CPU or supervisor
- Start address
- Length
- Direction
- Expiration or revocation rule
- Fault behavior

## Open Questions

- Final Rev A ROM size
- Final Rev A SRAM size
- Exact UART part
- Whether Rev A uses a 16 KiB or 8 KiB I/O aperture
- Whether ROM is mirrored for simple decoding
- Whether FPGA registers enter at Rev C only or appear in limited form in Rev B

<!-- AEMS-MIGRATED-SOURCE: docs/register-map-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/register-map-v1.md -->
### Integrated source: `docs/register-map-v1.md`

# ATARIX Register Map v1

## Status

Draft register specification for Rev B and Rev C development.

This document defines the first stable hardware/software contract between the W65C816, supervisor controller, FPGA fabric, diagnostics, and future operating-system services.

## Address Allocation

All registers reside within the primary I/O aperture.

```text
$00C000-$00C0FF UART
$00C100-$00C1FF GPIO
$00C200-$00C2FF Supervisor Mailbox
$00C300-$00C3FF Board Identity
$00D000-$00D0FF Fabric Control
$00D100-$00D1FF Interrupt Controller
$00D200-$00D2FF Mailbox Engine
$00D300-$00D3FF DMA Engine
$00D400-$00D4FF Timers
$00D500-$00D5FF Discovery Services
$00D600-$00D6FF Performance Counters
```

---

## UART Block

Base Address:

```text
$00C000
```

Registers:

```text
+00 DATA
+01 STATUS
+02 CONTROL
+03 BAUD
```

STATUS bits:

```text
Bit 0 RX Ready
Bit 1 TX Ready
Bit 2 Framing Error
Bit 3 Overrun
```

---

## Supervisor Mailbox

Base:

```text
$00C200
```

Registers:

```text
+00 COMMAND
+01 STATUS
+02 ARG0
+03 ARG1
+04 ARG2
+05 ARG3
```

Example commands:

```text
01 Reset Request
02 Recovery Mode
03 Read Board ID
04 Read Fault Log
05 FPGA Reload
```

---

## Board Identity Block

Base:

```text
$00C300
```

Fields:

```text
+00 Vendor ID Low
+01 Vendor ID High
+02 Board ID Low
+03 Board ID High
+04 Revision
+05 Capability Flags
```

---

## Fabric Control Block

Base:

```text
$00D000
```

Registers:

```text
+00 Fabric Version
+01 Fabric Status
+02 Fabric Control
+03 Fault Status
```

Fabric Control bits:

```text
Bit 0 Enable Fabric Services
Bit 1 Enable Discovery
Bit 2 Enable DMA
Bit 3 Enable Diagnostics
```

---

## Interrupt Controller

Base:

```text
$00D100
```

Registers:

```text
+00 Pending
+01 Mask
+02 Priority
+03 Acknowledge
```

Interrupt Sources:

```text
0 UART
1 Supervisor
2 Timer
3 Mailbox
4 DMA
5 Network
6 Accelerator
7 Reserved
```

---

## Mailbox Engine

Base:

```text
$00D200
```

Registers:

```text
+00 Mailbox Status
+01 Mailbox Control
+02 Message Length
+03 Source ID
+04 Destination ID
+05 Message Type
```

Mailbox payload storage is implementation dependent.

---

## DMA Engine

Base:

```text
$00D300
```

Registers:

```text
+00 Control
+01 Status
+02 Source Low
+03 Source High
+04 Destination Low
+05 Destination High
+06 Length Low
+07 Length High
```

Future revisions may expand to descriptor-based operation.

---

## Timer Block

Base:

```text
$00D400
```

Registers:

```text
+00 Counter Low
+01 Counter High
+02 Compare Low
+03 Compare High
+04 Control
```

---

## Discovery Services

Base:

```text
$00D500
```

Registers:

```text
+00 Device Count
+01 Selected Device
+02 Vendor ID Low
+03 Vendor ID High
+04 Device ID Low
+05 Device ID High
+06 Capability Pointer
```

Inspired by NuBus-style discovery concepts.

---

## Performance Counters

Base:

```text
$00D600
```

Registers:

```text
+00 Cycle Counter Low
+01 Cycle Counter High
+02 Interrupt Count
+03 DMA Count
+04 Mailbox Count
```

---

## Versioning Rule

New revisions may add registers.

Existing register meanings must not change without incrementing the major specification version.

<!-- AEMS-MIGRATED-SOURCE: docs/system-spec-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/system-spec-v1.md -->
### Integrated source: `docs/system-spec-v1.md`

# ATARIX System Specification v1

## Status

Draft engineering specification for Rev A and Rev B development.

This document serves as the authoritative reference until superseded by later revisions.

---

# 1. System Objectives

Rev A objectives:

- Boot a W65C816 successfully.
- Execute monitor firmware.
- Verify RAM.
- Verify ROM.
- Verify UART communications.
- Establish development workflow.

Rev B objectives:

- Introduce modularization.
- Add supervisor services.
- Add networking.
- Prepare for FPGA integration.

---

# 2. Processor

Primary CPU:

- Western Design Center W65C816S.

Operating modes:

- Native mode preferred.
- Emulation mode available for diagnostics.

Clock targets:

- Initial: 1 MHz debug clock.
- Normal bring-up: 4 MHz.
- Target operational range: 8–14 MHz depending on board implementation.

---

# 3. Memory Architecture

## Rev A Memory Map

```text
$000000-$0000FF Direct page
$000100-$0001FF Stack
$000200-$007FFF General RAM
$008000-$00BFFF Expansion window
$00C000-$00DFFF I/O window
$00E000-$00FFFF ROM
```

Vector table remains in ROM.

---

# 4. UART

Requirements:

- Console access
- Firmware upload
- Diagnostics

Candidate devices:

- 65C51-compatible UART
- FPGA UART implementation

Default rate:

- 115200 baud

---

# 5. Supervisor Controller

Responsibilities:

- Power sequencing
- Reset sequencing
- Recovery support
- Configuration storage
- Fault logging

Candidate families:

- RP2350
- STM32
- ESP32-S3

---

# 6. FPGA Fabric

Development target:

- ULX3S
- Lattice ECP5

Initial responsibilities:

- Address decode
- Interrupt routing
- Timers
- Mailboxes

Future responsibilities:

- DMA
- Discovery
- Capability enforcement assistance

---

# 7. Backplane

Initial slot types:

- CPU
- Service
- Fabric
- Accelerator

Design assumptions:

- Active backplane
- Supervisor visibility
- Slot isolation capability

---

# 8. Networking

Rev B networking:

- W5500 Ethernet

Functions:

- Firmware transfer
- Remote diagnostics
- Development support

---

# 9. Security Foundations

Principle:

Devices are not trusted by default.

Future systems should use:

- Capability grants
- DMA mediation
- Revocation mechanisms

---

# 10. Software Stack

Stage 1:

- ROM monitor
- Diagnostics

Stage 2:

- Loader
- Networking

Stage 3:

- Kernel
- Driver framework

Stage 4:

- Capability broker
- Multiprocessor services

Stage 5:

- Accelerator runtime

---

# 11. Deliverables

Rev A:

- Working CPU board
- ROM monitor
- Memory diagnostics
- UART console

Rev B:

- Supervisor services
- Ethernet support
- Modular architecture foundation

---

# 12. Open Engineering Decisions

- CPU card connector
- Backplane connector
- Final UART implementation
- Supervisor controller selection
- FPGA deployment strategy
- Register map finalization
- DMA architecture
- Discovery ROM format
- Capability encoding

This document establishes the baseline implementation target for ATARIX Rev A and Rev B.

<!-- AEMS-MIGRATED-SOURCE: docs/ulx3s-backplane-controller.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ulx3s-backplane-controller.md -->
### Integrated source: `docs/ulx3s-backplane-controller.md`

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
