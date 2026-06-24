# ATARIX Management Anomaly Detection v1

## Status

Draft management-plane architecture specification.

This document defines an advisory anomaly-detection layer for the ATARIX supervisor and management fabric. The intent is to help distinguish normal hardware behavior from abnormal behavior so watchdog and recovery actions are more appropriate.

## Purpose

A simple watchdog can detect that something stopped responding.

A management anomaly detector can help answer a better question:

```text
Is this behavior normal for this card, state, workload, and boot phase?
```

The system should use this information to improve fault classification, reset policy, and diagnostic logging.

## Design Rule

Anomaly detection is advisory.

It may influence watchdog policy, reset escalation, and fault classification, but it must not be the only mechanism that decides to reset hardware.

Hard safety limits remain deterministic.

## Design Goals

1. Learn normal heartbeat behavior.
2. Detect abnormal timing, voltage, temperature, reset, DMA, and fabric activity.
3. Improve watchdog reset decisions.
4. Reduce unnecessary resets during slow but valid operations.
5. Escalate faster when behavior clearly indicates a hang or unsafe condition.
6. Keep the algorithm simple enough for supervisor MCU implementation.
7. Preserve deterministic recovery paths.

## Inputs

Candidate management-plane inputs:

```text
CPU heartbeat interval
FPGA heartbeat interval
Slot heartbeat interval
Boot phase
Reset reason
Power rail state
Temperature readings
DMA active duration
Mailbox queue depth
Interrupt rate
Fabric fault status
Network boot status
RTC/NTP sync state
Slot identity
Card revision
Firmware version
```

## Bayesian Filtering Concept

A Bayesian or Bayesian-inspired filter may maintain probability estimates for states such as:

```text
Normal
Booting
Slow But Valid
Hung
Power Fault
Thermal Fault
DMA Fault
Fabric Fault
Communication Fault
Unknown
```

The model estimates the most likely state based on observed evidence.

Example:

```text
Long CPU heartbeat gap
+ DMA inactive
+ no mailbox progress
+ power good
+ same boot phase for too long
= likely CPU hang
```

Another example:

```text
Long CPU heartbeat gap
+ netboot active
+ network receive activity
+ DMA active within limit
= likely slow but valid boot activity
```

## Deterministic Limits

Some conditions should bypass probabilistic reasoning.

Examples:

```text
Power rail out of tolerance
Over-temperature limit exceeded
DMA active beyond hard timeout
Fabric fault asserted
Supervisor watchdog failure
Manual recovery request
```

These conditions should trigger predefined recovery actions.

## State Confidence

The anomaly detector should maintain a confidence value.

Candidate confidence bands:

```text
0-25%   Unknown / insufficient evidence
26-60%  Weak indication
61-85%  Probable
86-100% Strong indication
```

Recovery actions should require both:

```text
Observed fault condition
Policy threshold met
```

## Watchdog Policy Integration

The watchdog should support escalation levels:

```text
Level 0 Observe only
Level 1 Log anomaly
Level 2 Warn / assert soft fault
Level 3 Reset affected service
Level 4 Reset affected slot
Level 5 Reset fabric or CPU
Level 6 Enter recovery mode
```

The anomaly detector may modify escalation timing.

Examples:

```text
High confidence CPU hang -> escalate faster
Low confidence slow boot -> delay reset
Repeated same-slot fault -> escalate to slot quarantine
Power instability -> avoid repeated reboot loop
```

## Normal Behavior Profiles

The supervisor may maintain per-card profiles.

Profile fields:

```text
Card type
Card revision
Firmware version
Expected heartbeat interval
Expected boot duration
Expected idle temperature range
Expected active temperature range
Expected DMA maximum duration
Expected mailbox response range
Fault history
```

Profiles should be updateable but not trusted blindly.

## Learning Mode

A supervised learning mode may record normal behavior during known-good operation.

Learning mode should record:

```text
Boot duration
Heartbeat intervals
Normal interrupt rates
Normal DMA durations
Normal temperature range
Normal voltage range
Mailbox response times
```

Learning mode must be explicitly enabled.

The system should not silently relearn during fault conditions.

## Fault Classification

Fault classes:

```text
CPU Hang
Fabric Hang
DMA Timeout
Mailbox Timeout
Thermal Fault
Power Fault
Network Boot Fault
Clock Fault
Reset Loop
Slot Fault
Unknown Fault
```

Fault logs should include:

```text
Timestamp
Card identity
Boot phase
Observed values
Estimated state
Confidence
Action taken
Previous related faults
```

## Reset Correctness

The purpose of the filter is not just to reset hardware, but to reset the correct hardware.

Examples:

```text
Slot heartbeat missing but fabric healthy -> reset slot
Fabric heartbeat missing and multiple slots silent -> reset fabric
CPU heartbeat missing but supervisor and fabric healthy -> reset CPU card
Power rail unstable -> hold affected slots in reset
Repeated accelerator faults -> quarantine accelerator slot
```

## Quarantine Mode

Repeated faults may place a card or slot into quarantine.

Quarantine behavior:

```text
Hold slot in reset
Disable DMA grants
Disable accelerator execution
Mark slot unsafe in discovery table
Require manual or supervisor-authorized re-enable
```

## Implementation Notes

Rev A implementation may begin with a simple scoring system rather than a full Bayesian model.

Example scoring model:

```text
+3 missed heartbeat
+2 stuck boot phase
+2 DMA active too long
+1 mailbox queue not draining
-2 network boot active
-1 recent valid interrupt
-1 temperature normal
```

Later revisions may replace this with a formal Bayesian filter.

## Supervisor Resource Limits

The algorithm must be small enough for the supervisor MCU.

Avoid:

```text
Large dynamic memory allocation
Floating-point dependency
Unbounded history buffers
Opaque model behavior
```

Prefer:

```text
Integer counters
Fixed-size history windows
Explicit thresholds
Small probability tables
Explainable fault decisions
```

## Operator Visibility

The supervisor diagnostic console should expose:

```text
SHOW HEALTH
SHOW ANOMALIES
SHOW PROFILE <slot>
SHOW WATCHDOG
SHOW FAULTS
CLEAR PROFILE <slot>
LEARN PROFILE <slot>
QUARANTINE SLOT <slot>
RELEASE SLOT <slot>
```

## Design Principle

The management fabric should learn what normal looks like, but recovery must remain explainable and deterministic.

The system should never hide behind a mysterious model when resetting hardware.

## Open Questions

- Whether Rev A uses scoring only or a small Bayesian table.
- Where profiles are stored.
- Whether learned profiles are signed or checksummed.
- Whether anomaly state is exposed through the discovery system.
- Whether the FPGA fabric contributes hardware counters directly.
- Whether quarantine state persists across reboot.
- How manual override interacts with quarantine and recovery mode.