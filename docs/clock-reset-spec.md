# ATARIX Clock and Reset Specification

## Status

Draft hardware specification for clock generation, clock distribution, reset sequencing, supervisor control, and FPGA startup.

## Purpose

Clocking and reset behavior form the foundation of the ATARIX platform.

This specification establishes deterministic startup behavior for:

- W65C816 CPU cards
- Supervisor controller
- FPGA fabric
- Service modules
- Future accelerator cards

## Design Goals

1. Deterministic power-on behavior.
2. Recoverable startup sequence.
3. Debug-friendly operation.
4. Supervisor-controlled reset.
5. FPGA configuration before CPU release when required.
6. Future multi-CPU support.

## Clock Domains

Initial clock domains:

```text
Supervisor Clock Domain
CPU Clock Domain
FPGA Fabric Clock Domain
Peripheral Clock Domain
```

Future revisions may introduce accelerator-specific clock domains.

## Primary Oscillator

Recommended system reference:

```text
25 MHz
```

Reasons:

- Common FPGA reference frequency.
- Suitable for Ethernet timing derivation.
- Easy PLL generation.
- Good long-term availability.

## Supervisor Clock

The supervisor controller should start immediately after power stabilization.

Supervisor clock requirements:

```text
Independent of W65C816 startup
Independent of FPGA configuration status
Available during recovery mode
```

The supervisor must remain operational even if the CPU or FPGA fails.

## CPU Clock Generation

Initial supported CPU clocks:

```text
1 MHz Debug
4 MHz Bring-up
8 MHz Normal
14 MHz Experimental
```

Clock selection may be controlled by:

- Supervisor firmware
- DIP switches
- Configuration EEPROM
- Recovery mode settings

## Debug Clock Mode

Debug mode should support:

```text
Single Step
Slow Clock
Continuous Clock
```

The supervisor may optionally gate CPU clock generation during debugging.

## FPGA Clocking

FPGA clock sources:

```text
Reference Oscillator
PLL-Derived Clocks
Optional External Debug Clock
```

Fabric services should not assume CPU and FPGA clocks are identical.

Clock-domain crossing must be explicit.

## Reset Sources

Possible reset sources:

```text
Power-On Reset
Supervisor Reset
Manual Reset Button
Recovery Button
Watchdog Timeout
Software Reset Request
Fabric Fault Reset
```

## Reset Tree

Logical hierarchy:

```text
Power Good
    |
Supervisor Controller
    |
    +-- FPGA Reset
    |
    +-- CPU Reset
    |
    +-- Slot Reset
```

The supervisor is the root reset authority.

## Reset Sequencing

### Phase 1

Power rails stabilize.

### Phase 2

Supervisor exits reset.

### Phase 3

Supervisor validates power-good status.

### Phase 4

FPGA configured if required.

### Phase 5

Fabric services initialized.

### Phase 6

CPU released from reset.

### Phase 7

Boot monitor executes.

## Recovery Mode

Recovery mode may hold:

```text
CPU in Reset
DMA Disabled
Accelerators Disabled
```

while leaving:

```text
Supervisor Active
UART Active
FPGA Configuration Access Active
```

## Watchdog Architecture

Supervisor watchdog responsibilities:

- Detect stalled CPU.
- Detect failed boot sequence.
- Trigger recovery.
- Record reset cause.

Watchdog expiration should never immediately erase diagnostic information.

## Reset Cause Register

Recommended retained causes:

```text
Power-On Reset
Manual Reset
Recovery Request
Watchdog Reset
Software Reset
Fabric Fault
Unknown
```

The ROM monitor should display reset cause information during startup.

## Multi-CPU Considerations

Future systems may support:

```text
Global Reset
Per-CPU Reset
Per-Slot Reset
```

The supervisor should be capable of resetting a single CPU card without affecting the remainder of the system.

## Design Rules

1. The supervisor always starts first.
2. CPU release is intentional, never automatic.
3. DMA remains disabled during early boot.
4. Accelerators remain disabled during early boot.
5. Recovery mode must remain available after failed firmware updates.
6. Clock generation must remain observable during debugging.

## Open Questions

- Final oscillator frequency.
- Supervisor MCU selection.
- External clock input support.
- FPGA configuration storage location.
- Multi-CPU synchronization strategy.
- Watchdog timeout values.
- Whether clock gating occurs in supervisor hardware or FPGA fabric.