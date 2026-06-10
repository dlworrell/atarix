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