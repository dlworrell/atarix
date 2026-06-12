# Supervisor Observability Contract v1

## Purpose

This document defines the minimum observability, health, and safety responsibilities of the ATARIX supervisor for cards, backplane services, and early platform bring-up.

The supervisor is an independent witness. It must be able to observe and validate the platform even if the CPU is held in reset, stalled, or unavailable.

## Core Principles

```text
Measurement must not depend on the thing being measured.
Observability Is A Feature.
Recovery Is Mandatory.
Engineering Access Is A First-Class Feature.
Scientific Method Over Assumption.
```

## Supervisor Role

The supervisor should be able to answer:

```text
Is the card present?
Is the card powered correctly?
Is the card electrically healthy?
Is the card thermally safe?
Is the card clocked correctly?
Is the identity source valid?
Is the discovery source valid?
Is the card ready for normal participation?
If not, why?
```

## Minimum Card Health Checks

Each supervised card should expose enough telemetry for the supervisor to validate the following categories.

### Identity Health

Required checks:

- Identity EEPROM present
- Identity EEPROM readable
- Identity EEPROM CRC valid
- Identity policy accepted
- ID_PRESENT / ID_VALID / ID_ERROR state available

### Discovery Health

Required checks:

- Discovery ROM present, if advertised
- Discovery ROM readable
- Discovery header valid
- Discovery CRC valid
- Discovery record count sane
- Discovery policy accepted

Recommended logical states:

```text
DISCOVERY_PRESENT
DISCOVERY_VALID
DISCOVERY_ERROR
```

### Electrical Rail Health

Required checks where hardware supports measurement:

- card input voltage present
- local regulator output valid
- over-voltage not detected
- under-voltage not detected
- brownout not detected
- over-current not detected
- rail sequencing completed
- power-good asserted

Recommended monitored rails:

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

Actual rail set depends on card class.

### Current and Power Health

Recommended checks:

- input current
- per-rail current where practical
- inrush event count
- over-current latch state
- fuse or eFuse state
- power budget class
- card power enable state

### Thermal Health

Required checks where practical:

- board temperature
- regulator temperature
- FPGA temperature, if present
- CPU temperature, if measurable
- memory temperature, if present
- thermal warning threshold
- thermal critical threshold

Recommended states:

```text
THERMAL_OK
THERMAL_WARN
THERMAL_CRITICAL
```

### Clock and Timing Health

Required checks where practical:

- reference clock present
- fabric clock present
- CPU clock present
- clock frequency within tolerance
- missing-clock fault state
- PLL lock state
- reset release timing
- monotonic timer alive

### Reset and Watchdog Health

Required checks:

- reset asserted / deasserted state
- reset reason
- watchdog enabled
- watchdog timeout count
- last watchdog event
- CPU reset hold reason
- supervisor safe-mode reason

### Communication Health

Required checks:

- supervisor link to card alive
- mailbox link alive, if advertised
- fabric link alive, if present
- I2C / SPI sideband bus health
- timeout count
- bus fault detection where practical
- transaction error count

## Health State Model

Each card should have a supervisor-visible health state.

Recommended states:

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

A card may be identified and still restricted.

A card may be electrically present and still unsafe.

A card may be valid but not authorized for the current operating mode.

## Supervisor Validation Sequence

Recommended sequence:

```text
Power applied
    ↓
Card presence detect
    ↓
Rail sanity check
    ↓
Clock sanity check
    ↓
Identity EEPROM validation
    ↓
Discovery ROM validation
    ↓
Thermal sanity check
    ↓
Policy evaluation
    ↓
Normal participation allowed or restricted
```

Failure at any stage should produce:

- supervisor event log entry
- health state transition
- visible indication
- engineering console report
- recovery or restricted-mode state where appropriate

## Event Log Requirements

The supervisor should maintain an event log with at least:

```text
timestamp_us
slot_or_card_id
event_class
stage
result
error_code
raw_status
```

Example:

```text
00001234 us  SLOT 3  POWER       V3P3        PASS
00001241 us  SLOT 3  IDENTITY    CRC32       PASS
00001251 us  SLOT 3  DISCOVERY   HEADER      PASS
00001280 us  SLOT 3  POLICY      HOMEBREW    RESTRICTED
```

The timestamp should use the Fabric Monotonic Timer model where available.

## Engineering Console Commands

Recommended supervisor console commands:

```text
show card <slot>
show health <slot>
show identity <slot>
show discovery <slot>
show rails <slot>
show thermals <slot>
show clocks <slot>
show faults <slot>
test identity <slot>
test discovery <slot>
test mailbox <slot>
test rails <slot>
test watchdog <slot>
acknowledge faults <slot>
```

## Physical Observability

Cards should expose test points sufficient for independent validation.

Identity test points:

```text
TP_ID_SCL
TP_ID_SDA
TP_ID_PWR
TP_ID_GND
TP_ID_VALID
```

Power test points:

```text
TP_VIN
TP_5V
TP_3V3
TP_2V5
TP_1V8
TP_1V2
TP_VCORE
TP_GND
```

Clock / timing test points, where applicable:

```text
TP_REFCLK
TP_CPUCLK
TP_FABCLK
TP_TRIGGER_IN
TP_TRIGGER_OUT
```

## Visual Status Indication

Supervisor-controlled visual indication should exist for:

- identity state
- card health state
- fault state
- activity state

Recommended identity colors:

```text
OFF     not present
GREEN   valid and accepted
BLUE    valid homebrew
AMBER   valid but restricted / warning
RED     invalid or failed
```

Recommended health colors:

```text
GREEN   ready
AMBER   warning or restricted
RED     faulted
PURPLE  recovery / provisioning mode
```

The CPU shall not be the sole controller of safety or identity indicators.

## Safety Policy

The supervisor should prevent normal participation when card health checks fail.

Recommended failure handling:

```text
Rail fault:
    enter faulted or restricted state

Thermal critical:
    enter faulted or safe state

Clock fault:
    enter reset-hold or faulted state

Identity fault:
    platform-critical cards enter safe state; development cards enter engineering/provisioning state

Discovery fault:
    restrict service publication; do not grant capabilities

Mailbox fault:
    isolate endpoint in policy; log timeout
```

## Relationship to Identity EEPROM

The Identity EEPROM specification defines the identity record.

This document defines how the supervisor observes, validates, reports, and acts on identity state.

## Relationship to Discovery ROM

Discovery ROMs describe resources and services.

The supervisor should validate discovery data before the card participates in normal service enumeration.

Discovery failure must not prevent recovery diagnostics.

## Relationship to Instrumentation Service

The Instrumentation Service should expose supervisor-observed health, counters, and fault records to higher-level software.

Higher-level software may consume supervisor telemetry, but it must not be required for basic measurement or recovery.

## Non-Goals

This document does not define:

- exact ADC part numbers
- exact voltage thresholds
- exact thermal thresholds
- full engineering panel mechanical layout
- final Fabric CSR mapping

Those belong in card-specific hardware specifications and Fabric CSR specifications.
