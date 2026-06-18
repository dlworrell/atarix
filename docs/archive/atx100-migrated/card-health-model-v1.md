# Card Health Model v1

Status: Draft v1

## Purpose

This document defines the common ATARIX card health model.

The health model provides a shared vocabulary for supervisors, validation tools, provisioning tools, monitors, firmware, and operating systems.

The health model answers:

```text
Is this card safe?
Is this card ready?
If not, why?
```

## Architectural Principles

```text
Measurement must not depend on the thing being measured.
```

```text
Every subsystem shall be:

Discoverable
Observable
Testable
Instrumentable
Recoverable
```

## Overall Health States

```text
ABSENT
PRESENT_UNVALIDATED
IDENTITY_VALID
DISCOVERY_VALID
RESTRICTED
READY
WARNING
FAULTED
SAFE_MODE
RECOVERY_MODE
```

## State Definitions

### ABSENT

No card is detected.

### PRESENT_UNVALIDATED

A card appears physically present, but identity and health validation are incomplete.

### IDENTITY_VALID

Identity EEPROM validation succeeded.

### DISCOVERY_VALID

Discovery ROM validation succeeded.

### RESTRICTED

The card is valid but policy restricts participation.

Common causes:

- homebrew policy
- development mode
- insufficient trust
- missing capability authorization
- ring restriction

### READY

The card has passed required validation and is available for normal participation.

### WARNING

The card is operational but has a warning condition.

Examples:

- elevated temperature
- rail margin warning
- degraded link
- missing optional service

### FAULTED

The card has a fault that prevents normal operation.

### SAFE_MODE

The card is held in a safe operating state pending diagnosis or policy decision.

### RECOVERY_MODE

The card is in a recovery or provisioning workflow.

## Health Domains

Each card may report health in multiple domains.

Required domains where applicable:

```text
Identity Health
Discovery Health
Power Health
Current Health
Thermal Health
Clock Health
Reset Health
Watchdog Health
Communication Health
Fabric Health
CPU Health
Instrumentation Health
```

## Domain States

Recommended per-domain states:

```text
UNKNOWN
NOT_PRESENT
OK
WARNING
CRITICAL
FAULT
RECOVERING
DISABLED_BY_POLICY
```

## Power Health

Power health may include:

- input voltage present
- regulator output valid
- rail sequencing complete
- under-voltage state
- over-voltage state
- power-good state

Common rails:

```text
VIN_BACKPLANE
V5_CARD
V3P3_CARD
V2P5_CARD
V1P8_CARD
V1P2_CARD
VCORE_CARD
VBAT_OR_RTC
```

## Current Health

Current health may include:

- input current
- per-rail current
- inrush event count
- over-current state
- fuse or eFuse state
- power budget class

## Thermal Health

Thermal health may include:

- board temperature
- regulator temperature
- FPGA temperature
- CPU temperature
- memory temperature
- warning threshold
- critical threshold

## Clock Health

Clock health may include:

- reference clock present
- fabric clock present
- CPU clock present
- PLL lock state
- frequency within tolerance
- missing-clock state

## Reset and Watchdog Health

Reset and watchdog health may include:

- reset asserted state
- reset reason
- reset hold reason
- watchdog enabled
- watchdog timeout count
- last watchdog event

## Communication Health

Communication health may include:

- sideband bus state
- mailbox state
- fabric link state
- timeout count
- transaction error count

## CPU Health

CPU cards additionally use the CPU Observability Contract.

Common CPU health indicators:

```text
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

## Instrumentation Health

Instrumentation health may include:

- logic analyzer available
- trace buffer available
- protocol decoder available
- counter bank available
- trigger router available
- capture active
- capture fault

## Event Reporting

Health transitions should generate supervisor event log records.

Recommended fields:

```text
timestamp_us
slot_or_card_id
health_domain
previous_state
new_state
reason_code
raw_status
```

## Recovery Rule

Recovery must not depend on the failed subsystem.

The supervisor must retain enough visibility to distinguish:

- absent card
- invalid identity
- invalid discovery
- power fault
- thermal fault
- clock fault
- communication fault
- policy restriction
- revoked trust

## Ring Security Notes

Health observation is not the same as health control.

Passive health visibility may be delegated to lower rings by policy.

Health control, recovery entry, provisioning entry, and reset authority generally remain Ring -2 or Ring -1 functions.

## Related Documents

- supervisor-observability-contract-v1.md
- cpu-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- trust-model-v1.md
- ADR-RING-SECURITY-MODEL.md
