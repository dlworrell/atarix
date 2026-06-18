# CPU Observability Contract v1

Status: Draft v1

## Purpose

CPU cards are a special class of ATARIX resource.

A CPU card is both a platform participant and a potential point of failure.

Core requirement:

```text
A CPU card shall be diagnosable
when the CPU is not functioning.
```

## Architectural Principles

```text
Measurement must not depend on the thing being measured.
```

```text
CPU observability shall be
architecture-independent at the interface,
architecture-specific at the implementation.
```

```text
Any two views shall be sufficient
to diagnose failure of the third.
```

## Three Views of Reality

### Supervisor View

Provides:

- power state
- thermal state
- clock state
- reset state
- watchdog state
- boot progress
- fault state

### FPGA View

Provides:

- bus activity
- trace capture
- protocol decode
- trigger events
- timing analysis
- fabric interaction visibility

### CPU View

Provides:

- monitor state
- operating system state
- software diagnostics
- runtime telemetry

## Generic CPU State Model

```text
CPU_ABSENT
CPU_POWER_GOOD
CPU_CLOCK_GOOD
CPU_RESET_ASSERTED
CPU_RESET_RELEASED
CPU_FETCHING
CPU_EXECUTING
CPU_ENUMERATED
CPU_MONITOR_RUNNING
CPU_OS_RUNNING
CPU_FAULTED
```

These states shall be presented through a common interface regardless of CPU family.

## Required Supervisor Visibility

The supervisor shall expose:

```text
Power State
Temperature
Clock State
Reset State
Heartbeat
Watchdog State
Boot Milestone
Fault State
Last Fault
Reset Cause
```

## Required FPGA Visibility

The FPGA instrumentation layer shall support:

```text
Bus Activity Detection
Instruction Fetch Detection
Interrupt Activity Detection
DMA Activity Detection
Trigger Events
Trace Capture
Protocol Decode
Timing Analysis
```

The FPGA should be able to determine:

```text
No Bus Activity
Repeated Vector Fetch
Unexpected Reset
Bus Lockup
Missing Clock Activity
```

without CPU cooperation.

## Boot Milestones

Standardized milestones:

```text
0x01 ROM_START
0x02 RAM_OK
0x03 FABRIC_ATTACH
0x04 ENUMERATION_OK
0x05 MONITOR_LOADED
0x06 OS_LOADED
```

Future CPU families shall reuse the same milestone interface.

## Runtime Health

Supervisor-visible runtime metrics:

```text
Heartbeat Counter
Watchdog Counter
Exception Counter
Fault Counter
```

Recommended registers:

```text
CPU_BOOT_MILESTONE
CPU_LAST_FAULT
CPU_STATE
```

## W65C816 Adapter Requirements

Initial implementation shall support observation of:

```text
PHI2
RESB
IRQB
NMIB
ABORTB
RDY
BE
VPA
VDA
MLB
```

and:

```text
A[23:0]
D[7:0]
```

through FPGA instrumentation.

## Engineering Test Points

Recommended minimum test points:

```text
TP_CPUCLK
TP_RESET
TP_IRQ
TP_NMI
TP_ABORT
TP_VCORE
TP_GND
```

## Multi-CPU Support

The interface shall support:

```text
CPU0
CPU1
CPU2
...
```

without requiring CPU-family-specific management software.

## Validation Integration

CPU observability shall support:

- manufacturing validation
- bring-up validation
- recovery mode diagnostics
- field diagnostics
- automated validation profiles

The observability layer must remain useful when:

- the monitor is absent
- the operating system is absent
- the CPU is halted
- the CPU is faulted
- the CPU is held in reset

## Related Documents

- supervisor-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- cpu-validation-strategy-v1.md
- ADR-THREE-FABRIC-ARCHITECTURE.md
- ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md
