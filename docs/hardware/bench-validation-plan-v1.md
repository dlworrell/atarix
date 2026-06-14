# Hardware Bench Validation Plan v1

Status: Draft

## Purpose

This plan defines the first ATARIX physical validation sequence.

The goal is not to boot the W65C816 first.

The goal is to prove the Supervisor-to-Fabric architecture on the bench before expensive CPU-card integration begins.

Primary target:

```text
RP2350 Supervisor
        <->
ECP5 Fabric
```

## Architectural Requirements

Bench validation must preserve the ATARIX doctrines:

```text
Supervisor owns trust.
Fabric enforces trust.
Everything is a service.
Discovery advertises.
Directory describes.
Capabilities authorize.
No direct hardware access.
Addresses are implementation details.
Deny by default.
Knowledge is not authority.
```

A hardware test is not complete merely because communication works.

It must also avoid creating bypass paths around the service and capability model.

## Phase 0: Bench Infrastructure

Goal:

```text
Establish reliable programming, power, measurement, and recovery tools.
```

Verify:

```text
ULX3S powers reliably.
RP2350 powers reliably.
USB serial access works.
Programming chain works.
Logic analyzer captures expected signals.
Oscilloscope probes expected rails and clocks.
Bench supply current limits are set.
Grounding is known and stable.
```

Success criteria:

```text
Both devices can be reflashed repeatedly.
Power cycling is repeatable.
Serial console is available.
Logic analyzer capture is repeatable.
```

## Phase 1: Supervisor Bring-Up

Goal:

```text
RP2350 boots and owns basic machine-control responsibilities.
```

Tests:

```text
Heartbeat LED
Serial boot banner
Watchdog setup
Reset output assertion
Reset output release
Power-good input readback
Fault input readback
```

Measurements:

```text
Boot time
Reset pulse width
Watchdog timeout
Power-good debounce time
Heartbeat period
```

Success criteria:

```text
Supervisor can boot, report status, assert reset, release reset, and recover from watchdog timeout.
```

## Phase 2: Fabric Bring-Up

Goal:

```text
ECP5 configures and becomes visible to Supervisor.
```

Minimum Fabric register block:

```text
0x0000 FABRIC_ID
0x0004 FABRIC_VERSION
0x0008 BUILD_ID
0x000C STATUS
0x0010 SCRATCH0
0x0014 SCRATCH1
0x0018 HEARTBEAT
0x001C FAULT_STATUS
```

Tests:

```text
Configuration done indication
Version register read
Build register read
Scratch register write
Scratch register readback
Heartbeat register changes
Fault status read
```

Success criteria:

```text
Supervisor can read and write the minimal Fabric register block repeatedly without protocol failure.
```

## Phase 3: Supervisor-Fabric Mailbox Validation

Goal:

```text
Establish reliable packet transport between Supervisor and Fabric.
```

Candidate mailbox header:

```c
struct atarix_mailbox_header_v1 {
    uint16_t type;
    uint16_t length;
    uint32_t sequence;
    uint32_t crc32;
};
```

Initial message types:

```text
PING
PONG
READ_REGISTER
WRITE_REGISTER
STATUS_QUERY
STATUS_REPLY
RESET_REQUEST
FAULT_REPORT
DISCOVERY_RECORD_PUSH
CAPABILITY_EVALUATE
```

Tests:

```text
Send packet
Receive packet
Sequence increment
CRC validation
Timeout handling
Retry handling
Duplicate packet handling
Out-of-order packet handling
Oversized packet rejection
Malformed length rejection
```

Fault injection:

```text
Drop packet
Duplicate packet
Corrupt CRC
Corrupt length
Corrupt sequence
Reset Fabric mid-transaction
Reset Supervisor mid-transaction
Hold Fabric busy
Send unknown message type
```

Success criteria:

```text
100,000 packets exchanged with zero unhandled protocol failures.
All injected faults are detected and reported.
No malformed packet grants authority or causes unsafe hardware action.
```

## Phase 4: Discovery Hardware Validation

Goal:

```text
Transport and validate Discovery records over the Supervisor-Fabric link.
```

Synthetic records:

```text
UART Service
GPIO Service
RTC Service
Fabric Status Service
Supervisor Health Service
```

Tests:

```text
BEGIN record accepted
SERVICE record accepted
END record accepted
CRC checked
Enumeration works
Iteration works
Lookup by handle works
Duplicate handles rejected
Missing END rejected
Invalid length rejected
Unknown record handled safely
```

Security requirement:

```text
Discovery reveals existence only.
Discovery does not grant authority.
```

Success criteria:

```text
Hardware transport can carry Discovery records equivalent to the structures covered by CI tests.
Discovery-only access cannot operate a protected resource.
```

## Phase 5: Capability Policy Hardware Validation

Goal:

```text
Exercise Capability Policy through the hardware transport before CPU-card integration.
```

Synthetic principals:

```text
Supervisor
Fabric
Kernel
Driver
Service
Application
```

Tests:

```text
Valid capability allowed
Invalid capability denied
Owner mismatch denied
Ring violation denied
Delegation violation denied
Revoked capability denied
Expired capability denied
Unknown handle denied
Unsupported operation denied
```

Security validation:

```text
Discovery only        -> DENY
Directory only        -> DENY
Known handle only     -> DENY
Known address only    -> DENY
Valid capability      -> EVALUATE
Invalid capability    -> DENY
Revoked capability    -> DENY
Expired capability    -> DENY
```

Success criteria:

```text
The bench proves that Discovery and Directory do not bypass Capability Policy.
Fabric enforcement remains mandatory for protected operations.
```

## Phase 6: First CPU Card Integration

Goal:

```text
Connect the W65C816 CPU card only after Supervisor, Fabric, mailbox, Discovery, and Capability paths are validated.
```

Preconditions:

```text
Supervisor validated
Fabric register block validated
Mailbox transport validated
Discovery transport validated
Capability policy validation complete
Fault injection complete
```

Initial CPU-card tests:

```text
Reset hold
Reset release
Clock present
ROM visible locally
SRAM visible locally
Vector fetch
Diagnostic heartbeat
Fabric service request
```

Success criteria:

```text
CPU card interacts with ATARIX through service paths rather than direct global hardware access.
```

## Bench Issues

Recommended issue sequence:

```text
Bench-001 RP2350 Bring-Up
Bench-002 ECP5 Bring-Up
Bench-003 Supervisor-Fabric Mailbox
Bench-004 Discovery Record Transport
Bench-005 Capability Policy Hardware Validation
Bench-006 First CPU Card Integration
```

## Required Instruments

Minimum bench setup:

```text
Bench power supply with current limiting
USB serial access
Logic analyzer
Oscilloscope
Known-good USB cables
Known-good programming cables
ESD-safe work surface
Common ground reference
```

Recommended measurements:

```text
3.3 V rail stability
1.2 V FPGA core rail stability
Clock stability
Reset pulse width
Mailbox signal timing
SPI/UART/parallel timing depending on selected link
Current draw during configuration
Current draw during mailbox stress
```

## Exit Criteria

Hardware Validation Sprint 1 is complete when:

```text
RP2350 boots.
ECP5 configures.
Supervisor can reset Fabric.
Supervisor can read Fabric version.
Supervisor can write and read scratch registers.
Mailbox packet exchange works.
Fault injection is detected safely.
Discovery records can be transported.
Capability decisions can be exercised over the link.
Unauthorized access attempts are denied.
```

## Result

This plan turns the ATARIX architecture into measurable bench work.

The first physical milestone is reliable Supervisor-Fabric communication with security-preserving service abstraction.
