# ATARIX Timing and Observability Architecture v1

## Status

Draft architecture specification.

This document defines ATARIX timing domains, monotonic counters, RTC usage, watchdog timing, event counters, measurement timestamps, and correlation rules for diagnostics and performance analysis.

## Purpose

ATARIX must be diagnosable from the first hardware revision.

Timing and observability are not optional conveniences. They are part of the platform architecture.

The system must be able to answer:

```text
When did it happen?
How long did it take?
Why was it slow?
Did forward progress stop?
What subsystem failed first?
```

## Design Rule

RTC answers:

```text
When did it happen?
```

Monotonic timers answer:

```text
How long did it take?
```

Counters answer:

```text
Why was it slow?
```

No subsystem shall use RTC time for scheduling, timeout, watchdog, or performance calculations.

## Timing Domains

ATARIX defines four timing domains:

```text
RTC Time
Fabric Monotonic Time
CPU Cycle or Instruction Counters
Subsystem Event Counters
```

Each domain has a distinct purpose.

## RTC Time

RTC time is human-readable wall-clock time.

Uses:

```text
Fault log wall-clock timestamps
Maintenance records
Audit records
User-visible date and time
NTP synchronization state
```

RTC time may be corrected by:

```text
NTP
Manual setting
RTC battery replacement
Supervisor policy
```

Because RTC time can move forward or backward, it must not be used for watchdogs, deadlines, scheduling, latency measurement, or performance profiling.

## Fabric Monotonic Timer

The ECP5 fabric controller should expose a global monotonic timer.

Recommended Rev A format:

```text
u64 monotonic counter
1 MHz frequency
1 tick = 1 microsecond
```

Properties:

```text
Never runs backward while powered
Wrap time effectively irrelevant for practical operation
Readable by CPU firmware
Readable by supervisor where practical
Usable by fabric services
Usable by diagnostics
```

At 1 MHz, a 64-bit counter wraps after roughly 584,000 years.

## Relationship to Linux Jiffies

The ATARIX monotonic timer is conceptually similar to Linux jiffies because it provides an elapsed-time counter.

However, it should not be a scheduler-tick counter.

Instead, it should be a fixed-rate microsecond-scale monotonic counter.

This avoids ambiguity caused by variable tick rates.

## Fabric Timer CSR Model

The fabric CSR map should expose timer registers similar to:

```text
TIMER_LOW       Lower 32 bits of monotonic counter
TIMER_HIGH      Upper 32 bits of monotonic counter
TIMER_FREQ      Ticks per second, recommended 1000000
TIMER_SNAPSHOT  Optional latch / snapshot control
```

Multi-byte and multi-register access rules must follow:

```text
docs/data-model-and-endianness.md
docs/register-map-v1.md
```

64-bit reads should support snapshot semantics so software can read a stable value on an 8/16-bit CPU.

## CPU Counters

CPU cards may expose local counters.

Examples:

```text
CPU cycle counter
Instruction retire counter, if available
Interrupt entry counter
Wait-state counter
RDY stall counter
Fabric-access stall counter
```

For W65C816 Rev A, some counters may be implemented by CPU-card bridge logic rather than the CPU itself.

CPU counters are local performance counters, not the global system time base.

## Event Counters

Every major subsystem should expose event counters where practical.

Examples:

```text
Mailbox messages sent
Mailbox messages received
Mailbox errors
DMA descriptors submitted
DMA descriptors completed
DMA bytes moved
DMA faults
Interrupts delivered
Interrupts dropped
Discovery reads
Capability lookups
Capability failures
Network packets
Netboot retries
Sensor samples
Watchdog warnings
Slot resets
```

Counters should be wide enough to avoid frequent wrap.

Recommended width:

```text
u64 for long-running counters
u32 for small local counters where wrap is acceptable and documented
```

## Measurement Timestamping

All measurements, diagnostics, health records, traces, and fault logs shall support monotonic timestamps.

A measurement record should include:

```text
Source
Measurement type
Value
Unit
Monotonic timestamp
Optional RTC timestamp
Slot or resource ID
Context, if available
```

Example:

```text
Source:       Fluke 289
Measurement:  +5V rail
Value:        5.013 V
Tick:         123456789
Slot:         CPU-0
Context:      Netboot stage 2
```

The monotonic timestamp is the primary correlation key.

The RTC timestamp is for human interpretation.

## Watchdog Timing

Watchdogs should use monotonic time.

Each monitored component should expose:

```text
Heartbeat counter
Last progress tick
Current state
Last fault tick
Reset count
Fault count
```

Supervisor logic should evaluate:

```text
current_monotonic_tick - last_progress_tick
```

rather than comparing RTC time.

A component should be considered suspicious when heartbeat continues but forward progress stops.

## Forward Progress

Heartbeat is not enough.

A card or service can be alive but wedged.

Therefore, every significant subsystem should distinguish:

```text
Alive
Making progress
Idle by design
Busy
Degraded
Faulted
Timed out
Quarantined
```

Forward progress indicators may include:

```text
Completed transactions
Advanced queue pointer
Updated state machine
Processed interrupt
Completed DMA descriptor
Responded to mailbox command
```

## Instrumentation Timing

The engineering instrumentation path should timestamp samples with monotonic time where practical.

This applies to:

```text
Voltage samples
Current samples
Temperature samples
Fan tachometer samples
Bluetooth instrument samples
Logic analyzer trigger events
Scope trigger events
Pattern generator events
Fault captures
```

Measurements collected through RP2350 supervisor paths and ECP5 fabric paths should be correlated to a shared monotonic timeline where practical.

## RTC and NTP Interaction

NTP may update RTC time after network synchronization.

NTP must not alter monotonic time.

NTP must not reset watchdog deadlines.

NTP must not affect elapsed-time measurements.

The supervisor should record time state:

```text
Unknown
RTC only
NTP synchronized
Manually set
Recovered from fault log
```

## Precision Timing Expansion

Future revisions should reserve architectural room for:

```text
PPS input
10 MHz reference input
10 MHz reference output
GPSDO integration
IEEE-1588 PTP
External trigger input
External trigger output
```

These features are not required for Rev A, but the architecture should not preclude them.

## Engineering Panel Integration

Engineering-panel signals may include:

```text
Clock output
Trigger input
Trigger output
Logic analyzer trigger
Supervisor console timestamp command
Measurement capture control
```

All engineering-panel capture events should be timestamped where practical.

## Monitor Commands

Suggested monitor commands:

```text
TIME SHOW
TIMER SHOW
TIMER READ
COUNTERS SHOW
HEALTH SHOW
WATCHDOG SHOW
TRACE SHOW
MEASURE SHOW
LATENCY SHOW
```

## Testing Requirements

Testing should verify:

```text
Monotonic timer increments
Monotonic timer does not move backward
Stable 64-bit timer reads on W65C816
RTC adjustment does not affect monotonic timer
Watchdog uses monotonic time
Event counters increment correctly
Counter wrap behavior is documented
Measurement records include timestamps
Trace correlation works across subsystems
```

See:

```text
docs/testing-strategy.md
```

## Design Principle

A timestamped failure is easier to debug than an unobserved failure.

A subsystem that cannot report health, progress, latency, faults, and recovery status is incomplete.

Correlating events across subsystems is a primary design goal.