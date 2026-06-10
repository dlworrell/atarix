# ATARIX CPU Card Architecture v1

## Status

Draft hardware architecture specification.

This document defines the first-generation ATARIX CPU card architecture. It is informed by the Vega816 CPU Shim, CPU Buffer, DMA Controller, and Vector Pull Rewrite KiCad projects, but adapts those ideas for the ATARIX workstation-style fabric architecture.

## Purpose

The CPU card provides the primary W65C816 execution environment while isolating the raw CPU bus from the system backplane.

The CPU card should be independently bring-up capable, diagnosable, and serviceable.

## Design Rule

The W65C816 local bus stays on the CPU card.

The backplane sees a structured ATARIX fabric interface, not every raw W65C816 signal.

## Architectural Position

```text
W65C816 CPU
    |
Local CPU Bus
    |
Local ROM / SRAM / I/O decode
    |
CPU Shim and Buffer Logic
    |
CPU-Card Bridge FPGA or CPLD
    |
ATARIX Fabric Interface
    |
DIN41612 Backplane
```

## Design Goals

1. Preserve simple W65C816 bring-up.
2. Keep the CPU bus observable.
3. Avoid exposing the full raw CPU bus across the backplane.
4. Support Vega816-derived CPU shim and buffer concepts.
5. Support vector-pull rewrite or equivalent interrupt assist logic.
6. Support wait-state generation and CPU pausing through RDY.
7. Support local boot even when the wider fabric is unavailable.
8. Support fabric-mediated DMA, discovery, interrupts, and mailboxes.
9. Prepare for future secondary CPU cards.

## Required Functional Blocks

### W65C816 CPU

Primary processor:

```text
WDC W65C816S
```

Required CPU signals to preserve locally:

```text
A0-A23
D0-D7 or D0-D15 depending on board strategy
RWB
PHI2
RDY
IRQB
NMIB
ABORTB
RESB
VDA
VPA
VPB
MLB
BE
E
MX
```

The CPU card should expose these signals for diagnostics, but they are not all backplane signals.

### Local ROM

The CPU card should contain local boot ROM or a reliable ROM mapping path.

ROM responsibilities:

- Reset vector.
- ROM monitor.
- Serial/network loader support.
- Recovery diagnostics.
- Basic memory and fabric tests.

ROM may later become selectable between:

```text
Normal ROM
Golden ROM
Network-loaded image
Supervisor-selected image
```

### Local SRAM

The CPU card should provide local SRAM sufficient for:

- Stack.
- Direct page.
- Monitor work area.
- Upload buffer.
- Netboot staging.
- Diagnostics.

The first CPU card should be useful even if no external memory card exists.

### Bank Zero Isolation

Each CPU card should own its own bank-zero environment.

Bank zero includes:

```text
$000000-$00FFFF
```

This preserves:

- Direct page behavior.
- Stack behavior.
- Interrupt-vector assumptions.
- ROM monitor assumptions.
- Per-CPU local execution state.

Future multi-CPU systems must not require every CPU to share the same physical bank zero.

### CPU Shim / Bridge Logic

The CPU card should include shim or bridge logic inspired by Vega816.

Responsibilities:

- CPU bus observation.
- Address decoding assistance.
- Wait-state insertion.
- RDY control.
- Local versus fabric access decision.
- Vector Pull Rewrite support if implemented.
- Fabric request generation.
- Debug-state reporting.

This may be implemented with:

```text
Discrete logic for Rev A simplicity
CPLD
Small FPGA
```

The preferred long-term implementation is FPGA or CPLD-based.

### CPU Buffering

The card should buffer CPU bus signals where useful.

Goals:

- Improve signal integrity.
- Permit debug observation.
- Isolate CPU timing from downstream logic.
- Support controlled DMA or bridge access.

The Vega816 CPU Buffer project is the reference inspiration for this block.

### Vector Pull Rewrite

The CPU card should reserve support for Vector Pull Rewrite or an equivalent interrupt-assist mechanism.

Purpose:

- Detect W65C816 vector fetches.
- Support expanded interrupt source handling.
- Permit fabric interrupt-controller integration.
- Reduce interrupt dispatch ambiguity.

This should be documented further in:

```text
docs/vector-pull-rewrite-v1.md
```

### Fabric Bridge

The CPU-card bridge translates CPU activity into ATARIX fabric transactions.

Supported transaction types:

```text
Register Read
Register Write
Mailbox Send
Mailbox Receive
Discovery Read
Interrupt Acknowledge
DMA Request Submission
Fault Report Read
```

The bridge should be able to stall the CPU with RDY while a fabric transaction completes.

## Local Address Map

The CPU card should implement a minimal local map compatible with the Rev A memory map.

Candidate Rev A local map:

```text
$000000-$0000FF  Direct page
$000100-$0001FF  Stack
$000200-$007FFF  Local RAM / monitor workspace
$008000-$00BFFF  Local expansion or staging RAM
$00C000-$00DFFF  Local and fabric I/O aperture
$00E000-$00FFFF  ROM / vectors
```

Fabric windows may appear inside the I/O aperture or selected higher banks.

See:

```text
docs/memory-map.md
docs/address-space-architecture.md
```

## Fabric Interface

The CPU card should expose a fabric-facing interface rather than a raw CPU bus.

Required logical groups:

```text
Power
Ground
Clock Reference
Reset Control
Management Plane
Control Plane
Event Plane
Data Plane
Discovery Interface
Diagnostic Signals
```

The exact connector pinout is deferred to:

```text
docs/backplane-pinout-v1.md
```

## Network Support

For Rev A, Ethernet on the CPU card is acceptable and likely preferred.

Recommended early network controller:

```text
W5500 Ethernet Controller
```

Reason:

- Simplifies TCP/IP burden.
- Supports netboot.
- Supports NTP.
- Supports remote diagnostics.
- Reduces dependency on a separate service card during bring-up.

Later revisions may move networking to a service card.

See:

```text
docs/netboot-ntp-v1.md
```

## Diagnostic Requirements

The CPU card must be highly observable.

Required diagnostic access:

```text
CPU Clock
CPU Reset
RDY
IRQB
NMIB
ABORTB
RWB
VDA
VPA
VPB
MLB
ROM Chip Select
RAM Chip Select
I/O Chip Select
Fabric Access Active
Vector Pull Rewrite Active
Bank Zero Select
```

Recommended diagnostic access:

```text
Buffered address bus sample header
Buffered data bus sample header
Logic analyzer control-signal header
Power rail test points
Ground probe loops
CPU running LED
Wait-state LED
IRQ LED
Fault LED
```

See:

```text
docs/diagnostic-access-v1.md
```

## Reset and Clocking

The CPU card accepts reset control from the supervisor or backplane reset tree.

Required reset modes:

```text
Power-On Reset
Supervisor Reset
Manual Reset
Slot Reset
Recovery Hold-In-Reset
Software-Requested Reset
```

Clock modes:

```text
1 MHz Debug
4 MHz Bring-up
8 MHz Normal
14 MHz Experimental
Single-Step or Gated Debug Clock
```

See:

```text
docs/clock-reset-spec.md
```

## Interrupt Model

The CPU card receives normal IRQ/NMI/ABORT signaling from local logic and the fabric interrupt controller.

Expected behavior:

- IRQB handles normal maskable events.
- NMIB is reserved for severe or recovery-class events.
- ABORTB is reserved for future protection and bus-fault experiments.
- Vector Pull Rewrite may assist dispatch.

See:

```text
docs/interrupt-architecture-v1.md
```

## DMA Model

The CPU card does not grant unrestricted raw bus-master access to external cards.

DMA should be mediated by the fabric controller.

CPU-card-local DMA concepts from Vega816 may be useful, but long-term ATARIX DMA must follow the capability model.

See:

```text
docs/dma-engine-v1.md
docs/capability-model.md
```

## Discovery Record

The CPU card should expose a discovery record describing:

```text
Vendor ID
Device ID
CPU type
Card revision
ROM version
Local RAM size
Supported clock modes
Supported interrupt features
Supported fabric features
Diagnostic capability flags
```

See:

```text
docs/discovery-rom-format.md
```

## Management Plane

The CPU card should include a low-speed management interface to the supervisor card.

Candidate functions:

- CPU heartbeat.
- Reset reason reporting.
- Temperature or voltage reporting if sensors exist.
- ROM version readout.
- Fault-state reporting.
- Hold-in-reset support.

This management interface must remain useful even when the main CPU is not booting.

## Relationship to Vega816

Keep:

- CPU shim idea.
- CPU buffer idea.
- RDY-based wait-state and pause control.
- Vector Pull Rewrite idea.
- Bank-zero awareness.
- Explicit DMA request/grant thinking.

Modify:

- BB816 connector assumptions.
- Raw shared-bus multiprocessing.
- Direct DMA-channel wiring.
- Interrupt routing details.

Avoid:

- Exposing the full CPU bus as the long-term system bus.

See:

```text
docs/vega816-analysis.md
```

## Initial Rev A CPU Card Recommendation

The first ATARIX CPU card should contain:

```text
W65C816S
Local SRAM
Local ROM or flash
CPU-local bus buffer/shim logic
Small FPGA or CPLD bridge
W5500 Ethernet
UART console path
Diagnostic headers and test points
Discovery EEPROM or ROM block
DIN41612 backplane connector
```

## Open Questions

- Exact local SRAM size.
- Exact ROM or flash part.
- Whether the first bridge is CPLD, small FPGA, or discrete logic.
- Whether W5500 lives on Rev A CPU card or a service card.
- Exact management-plane electrical interface.
- Exact DIN41612 pin usage.
- Whether Vector Pull Rewrite is mandatory in Rev A or reserved for Rev B.
- Whether the CPU card includes its own oscillator or derives clock from the backplane.
- Whether local RAM is always bank zero or can be remapped by the bridge.

## Design Principle

The CPU card is not the whole computer.

It is a deterministic W65C816 execution node attached to a workstation-style ATARIX fabric.