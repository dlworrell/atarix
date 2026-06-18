# ATARIX Diagnostic Access v1

## Status

Draft hardware engineering requirement.

This document defines test-point, oscilloscope, logic-analyzer, jumper, LED, and diagnostic-access requirements for ATARIX cards and backplanes.

## Purpose

ATARIX is a staged hardware project. Early revisions must be easy to debug with ordinary bench tools.

Every major board should expose enough diagnostic access to allow bring-up, timing verification, signal-integrity checks, fault isolation, and firmware debugging without destructive probing.

## Design Goals

1. Make clock, reset, bus, interrupt, mailbox, DMA, and fabric activity observable.
2. Support oscilloscope probing without shorting adjacent pins.
3. Support logic-analyzer attachment.
4. Provide ground references close to critical test points.
5. Support single-step or slow-clock operation.
6. Preserve recovery access even when the CPU or fabric is not booting.

## Required Diagnostic Features

Each major board should include:

- Clearly labeled test points.
- Nearby ground test points.
- At least one logic-analyzer header where practical.
- Power-rail measurement points.
- Reset-state visibility.
- Clock visibility.
- Debug LEDs.
- Recovery or mode-selection jumpers/DIP switches where appropriate.

## Backplane Test Points

The backplane should expose test points for:

```text
+12V
+5V
+3.3V
+3.3V Standby
Ground
System Clock
Debug Clock
Global Reset
Slot Reset Lines
Fabric Ready
Supervisor Ready
Recovery Mode
```

The backplane should also expose diagnostic access to:

```text
Interrupt/Event Lines
Mailbox Control Lines
Discovery Bus
Slot Identification Lines
Fabric Transaction Signals
```

## CPU Card Test Points

CPU cards should expose:

```text
CPU Clock
CPU Reset
RDY
IRQ
NMI
ABORT
R/W
VDA
VPA
ML
VPB
A0-A23 sample header or buffered subset
D0-D7 / D0-D15 sample header or buffered subset
ROM Chip Select
RAM Chip Select
I/O Chip Select
Bank Zero Select
Vector Pull Rewrite Active
```

If the full address and data buses cannot be exposed, provide a buffered logic-analyzer header with at least the most useful debug signals.

## Fabric Controller Test Points

The ULX3S/ECP5 fabric-controller connection should expose:

```text
FPGA Clock
FPGA Reset
Configuration Done
Fabric Ready
Fabric Fault
Mailbox Pending
DMA Active
DMA Fault
Interrupt Pending
Discovery Active
Slot Scan Active
```

The fabric controller should also have:

- DIP-switch inputs.
- LED outputs.
- JTAG access.
- UART or console access where practical.

## Supervisor Card Test Points

The supervisor controller should expose:

```text
Supervisor Clock
Supervisor Reset
Power Good Inputs
CPU Reset Output
FPGA Reset Output
Slot Reset Outputs
Watchdog Output
Recovery Input
Fault Log Trigger
UART TX/RX
I2C/SPI Management Bus
```

The supervisor must remain debuggable even when CPU and FPGA cards are held in reset.

## Service and Accelerator Card Test Points

Service and accelerator cards should expose:

```text
Card Power Rails
Local Reset
Fabric Interface Clock
Fabric Interface Reset
Interrupt Output
Mailbox Activity
DMA Request
DMA Grant
Discovery ROM / EEPROM Access
Fault Output
```

Accelerator cards should additionally expose job/queue activity indicators where practical.

## Oscilloscope Probing Requirements

Critical analog and timing points should use test pads or loops large enough for oscilloscope probes.

Recommended points:

```text
Clock Outputs
Reset Edges
Power Rails
FPGA PLL Outputs if accessible
CPU Clock Input
RDY Timing
Interrupt Assertion
Chip Select Timing
```

Provide ground loops or ground pads near high-speed or timing-sensitive points.

## Logic Analyzer Headers

Where practical, use keyed 0.1 inch or 2.54 mm headers for low-speed logic analyzer access.

Recommended CPU-card logic-analyzer groups:

```text
Address Low
Address High
Data Bus
Control Signals
Interrupt Signals
Chip Selects
```

Recommended fabric-controller groups:

```text
Mailbox Signals
Interrupt/Event Signals
DMA Signals
Discovery Signals
Slot Control Signals
```

## LEDs

LEDs should be used for coarse state visibility.

Recommended backplane or fabric LEDs:

```text
Power Good
Supervisor Ready
FPGA Config Done
Fabric Ready
Recovery Mode
CPU Reset Released
DMA Active
Fault
```

Recommended CPU-card LEDs:

```text
CPU Running
ROM Access
RAM Access
I/O Access
IRQ Active
NMI Active
Wait State / RDY Held
```

## Jumpers and DIP Switches

Manual configuration should be available for early bring-up.

Candidate controls:

```text
Recovery Mode
Golden ROM Select
Golden FPGA Bitstream Select
Slow Clock Enable
Single Step Enable
DMA Disable
Fabric Disable
Supervisor Override
Slot Test Mode
```

## Documentation Requirement

Every test point, jumper, DIP switch, LED, and debug header must be documented in the board schematic and in the relevant board-specific document.

Silkscreen labels should match documentation labels.

## Design Rule

If a signal is essential to boot, reset, clocking, interrupt delivery, DMA, mailbox operation, or recovery, it should either be directly testable or observable through a documented diagnostic mechanism.

Debuggability is a first-class hardware requirement, not a cleanup task after layout.

## Open Questions

- Final logic-analyzer header standard.
- Whether to use 0.1 inch headers, Tag-Connect, or both.
- How many LEDs belong on the backplane versus on the ULX3S adapter.
- Whether address/data buses are fully exposed or sampled through FPGA debug registers.
- Probe-loop footprint standard.