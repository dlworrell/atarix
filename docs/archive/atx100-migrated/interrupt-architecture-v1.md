# ATARIX Interrupt Architecture v1

## Status

Draft interrupt architecture specification.

This document defines interrupt sources, routing, priority, masking, acknowledgement, supervisor interaction, and future multi-CPU delivery for ATARIX.

## Purpose

Interrupts connect the W65C816 CPU, supervisor controller, FPGA fabric, mailbox engine, DMA engine, timers, network services, and future accelerators.

The interrupt architecture must remain simple enough for Rev A bring-up while allowing a fabric-mediated interrupt controller in later revisions.

## Design Goals

1. Keep Rev A interrupt handling simple and observable.
2. Permit Rev C FPGA-mediated interrupt routing.
3. Support interrupt masking and prioritization.
4. Support mailbox and DMA completion events.
5. Prepare for future multi-CPU routing.
6. Preserve a deterministic recovery path.

## W65C816 Interrupt Context

The W65C816 supports:

```text
IRQ
NMI
ABORT
RESET
```

For ATARIX:

- RESET is controlled by the supervisor and board reset logic.
- NMI is reserved for high-priority fault or recovery events.
- IRQ is used for normal maskable device and fabric events.
- ABORT is reserved for future bus fault or protection experiments.

## Interrupt Classes

```text
Class 0 Reset / Boot Control
Class 1 Non-Maskable Fault
Class 2 Fabric Fault
Class 3 Timer
Class 4 Mailbox
Class 5 DMA
Class 6 Network
Class 7 Storage
Class 8 Accelerator
Class 9 External Slot Device
Class 10 Debug / Monitor
```

## Initial Interrupt Sources

```text
0 UART
1 Supervisor
2 Timer
3 Mailbox
4 DMA
5 Network
6 Accelerator
7 External Slot
8 Fabric Fault
9 Debug Request
```

These source numbers extend the initial interrupt list in `docs/register-map-v1.md` and should be treated as the preferred v1 source list.

## Interrupt Controller Register Block

See:

```text
docs/register-map-v1.md
```

Base:

```text
$00D100
```

Minimum registers:

```text
+00 Pending Low
+01 Pending High
+02 Mask Low
+03 Mask High
+04 Priority Select
+05 Acknowledge
+06 Route Target
+07 Status
```

The original four-register draft is considered the minimal compatibility subset.

## Pending Register

Each bit corresponds to an interrupt source.

```text
Bit 0 UART
Bit 1 Supervisor
Bit 2 Timer
Bit 3 Mailbox
Bit 4 DMA
Bit 5 Network
Bit 6 Accelerator
Bit 7 External Slot
Bit 8 Fabric Fault
Bit 9 Debug Request
```

## Mask Register

A set bit masks the corresponding interrupt source.

Masking prevents delivery to CPU IRQ but does not necessarily clear the pending bit.

## Priority Model

Initial priority order:

```text
Highest: NMI / Supervisor Fault
Fabric Fault
DMA Fault
Mailbox Fault
Timer
UART
Network
Storage
Accelerator Completion
External Slot
Lowest: Debug Request
```

The priority model may later become programmable.

## Acknowledge Model

Interrupts should be acknowledged explicitly.

Acknowledge sequence:

1. CPU enters interrupt handler.
2. CPU reads pending register.
3. CPU services highest-priority unmasked source.
4. CPU writes source number or bit mask to acknowledge register.
5. Controller clears pending status if the source is no longer asserted.

Level-triggered sources may reassert until the device condition is cleared.

## IRQ Routing

Rev A:

```text
Device IRQ lines may be directly ORed or simply decoded.
```

Rev C and later:

```text
FPGA fabric routes device events to CPU IRQ.
```

The FPGA interrupt controller should expose pending, mask, priority, and acknowledgement state.

## NMI Use

NMI is reserved for conditions requiring immediate attention.

Candidate NMI sources:

- Supervisor emergency fault.
- Fatal fabric fault.
- Recovery button.
- Watchdog expiration.

NMI must not be used for routine device completion.

## ABORT Use

ABORT is reserved for future protection and bus-error experiments.

Possible future uses:

- Invalid memory window access.
- Capability violation.
- Bus timeout.
- Fabric protection fault.

ABORT should remain disabled or unused until the system can report faults reliably.

## Mailbox Interrupts

Mailbox interrupts indicate:

- Message received.
- Response available.
- Mailbox fault.

Mailbox handlers should read mailbox status before reading payload data.

See:

```text
docs/mailbox-protocol-v1.md
```

## DMA Interrupts

DMA interrupts indicate:

- Transfer complete.
- Transfer fault.
- Capability validation failure.
- Timeout.

See:

```text
docs/dma-engine-v1.md
```

## Timer Interrupts

Timer interrupts support:

- Monitor delays.
- Scheduler ticks.
- Watchdog service.
- Benchmarking.

Timer interrupts should be maskable.

## Supervisor Interrupts

Supervisor-generated interrupts may indicate:

- Reset reason available.
- Fault log update.
- Recovery request.
- Power or thermal warning.
- Configuration event.

Supervisor emergency events may use NMI depending on severity.

## Multi-CPU Routing

Future Rev D systems may route interrupts to specific CPU cards.

Routing targets:

```text
$00 Broadcast
$01 Primary CPU
$02 Secondary CPU
$03 Supervisor
$04 Fabric Service
```

Future routing policy may include:

- Fixed CPU ownership.
- Round-robin delivery.
- Priority CPU delivery.
- Supervisor-directed routing.

## Fault Reporting

Interrupt-related faults should report:

```text
Source
Class
Pending Mask
Delivery Target
Fault Code
Timestamp or Counter
```

## Boot Defaults

On reset:

```text
All maskable interrupts disabled
Pending state cleared if safe
NMI reserved for recovery/fault
DMA interrupts disabled until DMA initialized
Mailbox interrupts disabled until mailbox initialized
Timer interrupts disabled until monitor enables them
```

## Design Rule

Interrupts notify.

They do not transfer ownership, grant authority, or imply trust.

Authority remains controlled by the capability model.

## Open Questions

- Final number of interrupt sources.
- Whether priorities are fixed or programmable.
- Whether interrupt status should be 16-bit or 32-bit.
- Whether ABORT should be used for capability faults.
- Multi-CPU delivery policy.
- Exact relationship between fabric faults and NMI.