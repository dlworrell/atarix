# ATARIX Supervisor Card Architecture v1

## Status

Draft hardware architecture specification.

This document defines the first-generation ATARIX supervisor card. The supervisor card implements the independent management plane for the ATARIX workstation architecture.

## Purpose

The supervisor card keeps the system observable, recoverable, and serviceable even when the W65C816 CPU, FPGA fabric controller, service cards, or accelerator cards have failed or are held in reset.

The supervisor is not a replacement for the CPU or fabric controller. It is a management and recovery authority.

## Design Rule

The supervisor must remain operational when the CPU and fabric are not booting.

It owns recovery, reset sequencing, watchdogs, RTC timekeeping, power-state monitoring, and fault logging.

## Architectural Position

```text
RTC / NVRAM / Fault Log
        |
Supervisor MCU
        |
Management Plane
        |
CPU Card / ULX3S Fabric Controller / Backplane Slots
```

The supervisor participates in the management plane, not the primary data plane.

## Design Goals

1. Provide deterministic reset sequencing.
2. Provide RTC-backed timekeeping.
3. Timestamp fault logs.
4. Monitor CPU, FPGA, and slot heartbeats.
5. Reset hung cards or the full system.
6. Support golden ROM and golden FPGA recovery.
7. Expose diagnostic information over UART or management interface.
8. Remain independent from the main CPU and FPGA fabric.
9. Support NTP-corrected RTC updates when the system network is available.

## Required Functional Blocks

### Supervisor MCU

Candidate MCU families:

```text
RP2350
RP2040
STM32
ESP32-S3
```

Preferred attributes:

- Multiple GPIOs.
- I2C/SPI/UART support.
- Nonvolatile configuration support.
- Watchdog timers.
- Low-power standby capability.
- Good open toolchain support.

### RTC

The supervisor card should include a battery-backed or supercapacitor-backed RTC.

Responsibilities:

- Maintain time when the system is powered off.
- Timestamp watchdog resets.
- Timestamp recovery-mode entries.
- Timestamp slot failures.
- Provide fallback time when NTP is unavailable.

NTP may correct the RTC at boot, but NTP does not replace the RTC.

See:

```text
docs/netboot-ntp-v1.md
```

### Fault Log Storage

The supervisor should include nonvolatile or retained fault-log storage.

Candidate storage:

```text
MCU internal flash
External SPI flash
I2C EEPROM
FRAM
Battery-backed SRAM
```

FRAM is attractive for repeated fault logging because it tolerates many writes.

Fault log entries should include:

```text
Timestamp
Fault source
Fault class
Reset cause
Slot number
Last heartbeat state
Power-good state
Fabric status if available
CPU status if available
```

### Watchdog Timers

The supervisor should monitor heartbeats from:

```text
Primary CPU Card
ULX3S Fabric Controller
Service Cards
Accelerator Cards
Future CPU Cards
```

Watchdog action levels:

```text
Level 0 Log warning
Level 1 Assert slot fault
Level 2 Reset offending slot
Level 3 Reset fabric controller
Level 4 Reset CPU card
Level 5 Enter recovery mode
```

Not every fault should trigger a full system reset.

### Reset Controller

The supervisor should control:

```text
Global Reset
CPU Slot Reset
Fabric Controller Reset
Per-Slot Reset
Recovery Reset
Manual Reset Input
Watchdog Reset Output
```

The reset controller must support holding selected slots in reset while allowing the supervisor to continue operating.

### Power Monitoring

The supervisor should monitor:

```text
+12V
+5V
+3.3V
+3.3V Standby
Optional local rails
```

Power faults should be logged before reset or shutdown if possible.

### Recovery Inputs

Required recovery controls:

```text
Recovery Button
Recovery DIP Switch
Golden ROM Select
Golden FPGA Bitstream Select
Fabric Disable
DMA Disable
Slow Clock / Debug Clock Enable
```

These may be physically located on the supervisor card, backplane, or ULX3S carrier, but the supervisor should be able to read their state.

## Management Plane

The supervisor communicates with system components over a low-speed management interface.

Candidate electrical interfaces:

```text
I2C
SPI
UART multidrop
RS-485 multidrop
CAN
Custom low-speed management bus
```

The management plane should be independent of the main ATARIX data plane.

Management-plane functions:

```text
Heartbeat collection
Slot identity readout
Reset control
Fault reporting
RTC read/write
Configuration read/write
Power-state reporting
Recovery-mode coordination
```

## Heartbeat Model

Each active card should provide a heartbeat.

Heartbeat states:

```text
Absent
Held In Reset
Booting
Alive
Warning
Faulted
Timed Out
```

A missing heartbeat should not immediately cause a reset unless the card is expected to be active.

## RTC and NTP Interaction

The CPU or network service may obtain NTP time during boot.

The supervisor should accept RTC updates only if:

```text
NTP result is valid
Time passes sanity checks
Update is authorized by boot policy
Time is newer than firmware build baseline
```

The supervisor should record whether time is:

```text
Unknown
RTC Only
NTP Synchronized
Manually Set
```

## Golden Recovery

The supervisor should support recovery modes:

```text
Golden ROM Boot
Golden FPGA Bitstream
CPU Held In Reset
Fabric Minimal Mode
Network Recovery Boot
UART Monitor Recovery
```

A failed firmware or FPGA update must not permanently brick the system.

## Diagnostic Interface

The supervisor should expose a diagnostic console.

Candidate interfaces:

```text
UART header
USB serial
Management bus command interface
Fabric mailbox endpoint after fabric startup
```

Minimum diagnostic commands:

```text
STATUS
RESET CPU
RESET FABRIC
RESET SLOT <n>
SHOW FAULTS
CLEAR FAULTS
SHOW RTC
SET RTC
SHOW POWER
SHOW HEARTBEATS
ENTER RECOVERY
```

## Integration With Boot Sequence

Supervisor startup occurs before CPU reset release.

Recommended sequence:

```text
Power stable
Supervisor boots
Power rails verified
Recovery inputs read
FPGA mode selected
Fabric reset controlled
CPU reset held
Fabric ready checked if required
CPU reset released
Boot monitored by heartbeat
```

See:

```text
docs/boot-sequence-v1.md
docs/clock-reset-spec.md
```

## Diagnostic Requirements

Required test points:

```text
Supervisor Clock
Supervisor Reset
RTC I2C/SPI Lines
Power Good Inputs
CPU Reset Output
Fabric Reset Output
Slot Reset Outputs
Watchdog Output
Recovery Input
Fault Log Write Indicator
Management Bus Lines
UART TX/RX
```

Required LEDs:

```text
Supervisor Alive
Power Good
Recovery Mode
CPU Reset Asserted
Fabric Reset Asserted
Fault Logged
Management Activity
```

See:

```text
docs/diagnostic-access-v1.md
```

## Security and Authority

The supervisor is the root management authority.

It may:

- Hold cards in reset.
- Select recovery paths.
- Record faults.
- Clear or preserve configuration.
- Authorize RTC updates.

It should not silently perform normal data-plane work.

The supervisor should not grant unrestricted DMA or accelerator access. Those remain governed by the fabric and capability model.

## Relationship to Fabric Controller

The ULX3S/ECP5 fabric controller owns main fabric services.

The supervisor owns management and recovery.

The supervisor may reset or configure the fabric controller, but it should not replace the fabric controller during normal operation.

See:

```text
docs/ulx3s-backplane-controller.md
docs/fpga-fabric.md
```

## Initial Rev A Supervisor Recommendation

The first supervisor card should include:

```text
MCU
RTC
FRAM or EEPROM fault log
Reset-control outputs
Power-good inputs
Recovery DIP/button inputs
UART/USB diagnostic console
Management bus interface
Status LEDs
Test points
```

## Open Questions

- Final MCU selection.
- RTC part selection.
- Fault-log storage technology.
- Management-bus electrical standard.
- Whether supervisor is a separate card or integrated into the backplane for Rev A.
- Number of independently resettable slots.
- Whether the supervisor directly loads FPGA bitstreams or only selects boot mode.
- Whether NTP updates are pushed by CPU firmware or pulled through a supervisor network path.
- Exact fault-log binary format.

## Design Principle

The supervisor is the system's service processor.

It exists so ATARIX can fail visibly, recover deliberately, and retain enough information to diagnose what happened.