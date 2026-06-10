# ADR-0002: RP2350 Supervisor Selection

Status: Accepted

Date: 2026-06-10

## Context

ATARIX needs an always-available supervisor controller for reset sequencing, watchdogs, RTC coordination, fault logging, recovery-mode control, power monitoring, and management-plane communication.

The supervisor must remain operational when the W65C816 CPU card or the FPGA fabric controller is not booting.

## Decision

Use the RP2350 as the Rev A supervisor MCU target.

## Evidence

- Strong GPIO, timer, and peripheral support.
- More headroom than RP2040 for watchdog policy, anomaly scoring, management commands, and future cryptographic checks.
- Good toolchain availability.
- Good community and documentation ecosystem.
- Suitable for deterministic low-speed management functions.

## Alternatives Considered

- RP2040.
- STM32 family.
- ESP32-S3.
- Small CPLD-only supervisor.
- Supervisor implemented inside the FPGA fabric.

## Consequences

Pros:

- Keeps management independent of the main CPU and FPGA fabric.
- Gives the supervisor enough compute headroom for health scoring and fault handling.
- Keeps Rev A implementation accessible and inexpensive.

Cons:

- Adds another firmware target.
- Requires explicit management-plane protocol design.
- Requires careful authority separation between supervisor and fabric controller.

## Future Revisions

Future boards may replace the RP2350, but the logical supervisor role should remain stable.