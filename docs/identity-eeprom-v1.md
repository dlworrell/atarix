# Identity EEPROM v1

## Purpose

The ATARIX Identity EEPROM is the root of hardware identity for every card in the system.

It answers:

```text
Who am I?
What am I?
Who built me?
How was I provisioned?
Can my identity record be validated?
How much trust should the platform assign me?
```

The Identity EEPROM is mandatory on every ATARIX card.

A board shall not be considered operational until its identity source has been validated by the supervisor.

## Core Principle

```text
Identity proves existence.
Trust determines authority.
Capabilities determine access.
```

Identity, trust, and capabilities are separate architectural concepts.

A valid EEPROM proves that a card can identify itself. It does not automatically grant dangerous capabilities such as DMA, boot authority, recovery authority, supervisor authority, or memory-controller authority.

## Scope

This specification defines:

- required physical identity storage
- required binary identity block
- validation rules
- manufacturing and provisioning fields
- trust-class model
- homebrew card support
- required observability signals
- required engineering test points
- recovery behavior

Detailed service capabilities are defined by the future Capability Record Format specification.

## Physical Device

Rev A should use a simple nonvolatile serial EEPROM.

Recommended class:

```text
I2C EEPROM
24AA256 / 24LC256 / AT24C256 or equivalent
Minimum capacity: 32 KiB
```

Rationale:

- inexpensive
- widely available
- simple to read from supervisor firmware
- large enough for future metadata
- adequate for provisioning records, signatures, and service history extensions

## Endianness and Data Model

All numeric fields are little-endian and follow `docs/data-model-and-endianness-v1.md`.

Unqualified addresses are forbidden in this specification.

Identity EEPROM records may contain handles or identifiers, but the mandatory identity block does not contain CPU-local addresses or Fabric addresses.

## Mandatory Identity Block

The first 512 bytes of the EEPROM are reserved for the mandatory identity block.

Future extension blocks may follow after the first 512 bytes.

## Binary Layout

```text
Offset  Size  Field
------  ----  --------------------------------
0x0000  4     Magic
0x0004  2     EEPROM format version
0x0006  2     Header length
0x0008  4     Total record length
0x000C  4     Vendor ID
0x0010  4     Device ID
0x0014  8     Serial number
0x001C  2     Board revision
0x001E  2     Board class
0x0020  2     Trust class
0x0022  2     Reserved / alignment
0x0024  8     Manufacture timestamp
0x002C  4     Boot firmware revision
0x0030  4     Provisioning tool revision
0x0034  8     Capability flags
0x003C  32    Board name
0x005C  4     Provisioning revision
0x0060  8     Provisioning timestamp
0x0068  8     Factory test signature
0x0070  4     CRC32
0x0074  396   Reserved for v1-compatible expansion
```

Total mandatory block size:

```text
512 bytes
```

## Magic

The magic field shall contain:

```text
ATIX
```

ASCII bytes:

```text
0x41 0x54 0x49 0x58
```

## Format Version

Initial version:

```text
0x0001
```

Breaking changes require a new format version.

Compatible extensions should use reserved space or extension blocks after the mandatory 512-byte identity block.

## Header Length

For v1:

```text
0x0200
```

This is the length of the mandatory identity block in bytes.

## Total Record Length

The total record length describes the total identity record area used in the EEPROM.

For a v1 record with no extensions:

```text
0x00000200
```

## Vendor ID

Vendor IDs are 32-bit identifiers.

Reserved values:

```text
0x00000000  Invalid / unprogrammed
0x00000001  ATARIX reserved
0xFFF00000-0xFFFFFFFF  Homebrew / experimental range
```

The homebrew range allows user-built cards without requiring official vendor allocation.

## Device ID

Device IDs are 32-bit vendor-assigned identifiers.

A Device ID is meaningful only within the Vendor ID namespace.

## Serial Number

Serial numbers are 64-bit vendor-assigned values.

Homebrew cards may use:

- user-assigned serial numbers
- generated serial numbers
- locally administered serial numbers

Serial numbers should be unique within a Vendor ID namespace.

## Board Revision

Board revision is a 16-bit value.

Recommended convention:

```text
major << 8 | minor
```

Example:

```text
0x0100  Rev 1.0
0x0101  Rev 1.1
```

## Board Class

Initial board classes:

```text
0x0000  Invalid / unknown
0x0001  CPU card
0x0002  Memory card
0x0003  Network card
0x0004  Storage card
0x0005  Supervisor card
0x0006  Backplane controller
0x0007  Development card
0x0008  Homebrew card
0x0009  Prototype card
0x000A  Unprovisioned card
0xFFFF  Vendor-specific
```

Board class describes what the board is.

Board class does not define what the board is authorized to do.

## Trust Class

Trust class describes the initial trust level assigned by provisioning policy.

Initial trust classes:

```text
0x0000  Invalid
0x0001  Unknown
0x0002  Homebrew
0x0003  Development
0x0004  Factory
0x0005  Platform critical
0xFFFF  Vendor-specific
```

Trust class is not a capability grant.

The platform may reduce effective trust based on policy, signature failure, missing provisioning records, development-mode switches, or supervisor configuration.

## Manufacture Timestamp

Manufacture timestamp is a 64-bit unsigned integer.

Recommended representation:

```text
microseconds since Unix epoch
```

If unavailable, set to zero.

## Boot Firmware Revision

Boot firmware revision identifies firmware associated with the card at provisioning time.

Recommended convention:

```text
major << 24 | minor << 16 | patch
```

## Provisioning Tool Revision

Provisioning tool revision identifies the tool version that wrote or last validated the identity record.

This is intentionally separate from boot firmware revision.

## Capability Flags

Capability flags are a compact summary of broad capability classes.

Detailed capabilities are described by Capability Records.

Initial bit assignments:

```text
Bit 0   Mailbox endpoint present
Bit 1   Discovery ROM present
Bit 2   DMA-capable hardware present
Bit 3   Interrupt-capable hardware present
Bit 4   Boot-capable hardware present
Bit 5   Recovery-capable hardware present
Bit 6   Supervisor-facing hardware present
Bit 7   Fabric memory participant
Bit 8   Network-capable hardware present
Bit 9   Storage-capable hardware present
Bit 10  Instrumentation-capable hardware present
Bit 11  Development / experimental hardware
Bits 12-63 reserved
```

A set bit advertises that hardware exists. It does not grant authority.

Authority is granted by policy and capability evaluation.

## Board Name

Board name is a fixed 32-byte UTF-8 field.

Rules:

- NUL-terminated if shorter than 32 bytes.
- Not required to be unique.
- Intended for engineering consoles and human-readable inventory.

Example:

```text
ATARIX Rev A CPU Card
```

## Provisioning Revision

Provisioning revision identifies the provisioning schema or process revision.

## Provisioning Timestamp

Provisioning timestamp is a 64-bit unsigned integer.

Recommended representation:

```text
microseconds since Unix epoch
```

If unavailable, set to zero.

## Factory Test Signature

Factory test signature is a 64-bit value written after manufacturing validation.

It may be a simple test signature in Rev A and may later become a handle or digest associated with a richer factory-test record.

Recommended initial states:

```text
0x0000000000000000  Not tested / unknown
0x4154495854455354  ATIXTEST / factory test passed marker
```

## CRC32

CRC32 validates the mandatory identity block.

Rules:

- CRC field is treated as zero during calculation.
- CRC covers bytes `0x0000` through `0x01FF` of the mandatory identity block.
- Algorithm: CRC-32/ISO-HDLC unless superseded by a later revision.

## Validation Chain

A card is considered operational only if all of the following succeed:

```text
Physical presence
Communication
Header validation
CRC validation
Identity validation
Policy evaluation
```

Supervisor validation states:

```text
ID_PRESENT
ID_READABLE
ID_HEADER_VALID
ID_CRC_VALID
ID_IDENTITY_VALID
ID_POLICY_ACCEPTED
ID_VALID
ID_ERROR
```

`ID_VALID` shall be true only when the supervisor has validated the identity source and policy accepts the card for the current operating mode.

## Supervisor Responsibilities

The supervisor shall:

1. Detect EEPROM physical presence where hardware permits.
2. Read the mandatory identity block.
3. Validate magic and format version.
4. Validate header length and total record length.
5. Validate CRC32.
6. Validate Vendor ID, Device ID, Board Class, and Trust Class against policy.
7. Record validation state.
8. Expose validation state through the engineering console.
9. Expose validation state to the Fabric / Backplane BIOS as appropriate.
10. Drive identity status signals and visual indication.

The CPU shall not be the only entity capable of validating identity.

## CPU Reset Policy

For platform-critical boards, the supervisor may hold CPU reset if identity validation fails.

Recommended default:

```text
Factory / Platform Critical failure:
    hold CPU reset or enter safe mode

Development / Homebrew failure:
    enter engineering or restricted mode

Unprovisioned card:
    enter provisioning mode if allowed
```

## Required Signals

Every ATARIX card shall expose or report the following logical identity states:

```text
ID_PRESENT
ID_VALID
ID_ERROR
```

These may be physical sideband signals, supervisor GPIOs, Fabric-readable status bits, or a combination of these mechanisms.

The signal source must not depend solely on the main CPU.

## Required Test Points

Rev A hardware should expose:

```text
TP_ID_SCL
TP_ID_SDA
TP_ID_PWR
TP_ID_GND
TP_ID_VALID
```

These test points allow validation with:

- DMM
- oscilloscope
- logic analyzer
- Bus Pirate-class tool
- RP2350 diagnostics

The engineering goal is that identity communication and validation can be observed without disassembling the system beyond normal service access.

## Visual Status Indicator

Identity status shall be visible to an engineer.

Recommended supervisor-controlled indication:

```text
OFF     No EEPROM detected
GREEN   Valid factory / trusted identity
BLUE    Valid homebrew identity
AMBER   Valid but restricted or warning state
RED     Invalid identity or communication failure
```

The CPU shall not be the sole controller of this indicator.

## Homebrew Card Support

Homebrew cards are supported as first-class citizens.

Homebrew support is not implicit trust.

A homebrew card may participate in the system if:

```text
EEPROM present
EEPROM readable
CRC valid
Board class is homebrew, prototype, or development
Capability flags are explicit
Policy allows the card in the current mode
```

Default restrictions for homebrew cards:

```text
DMA disabled
Boot authority disabled
Recovery authority disabled
Supervisor authority disabled
Memory-controller authority disabled
Interrupt-controller authority disabled unless explicitly granted
```

Homebrew cards may expose safer services such as:

- GPIO
- serial
- LEDs
- sensors
- simple mailbox endpoints
- instrumentation outputs

Privileged access requires explicit provisioning and policy approval.

## Development Mode

ATARIX hardware may provide a development-mode jumper or switch.

Example names:

```text
JP_DEV_MODE
SW_DEV_ENABLE
```

When asserted, policy may allow:

- homebrew cards
- experimental EEPROMs
- verbose diagnostics
- relaxed provisioning requirements

Development mode must not silently grant unrestricted DMA, supervisor authority, boot authority, or recovery authority.

## Engineering Console Requirements

The engineering console should expose identity status.

Recommended command:

```text
show identity
```

Example output:

```text
Slot: 3
Board: Homebrew Sensor Card
Vendor ID: 0xFFF01234
Device ID: 0x00000001
Serial: 0x0000000000000001
Board Class: Homebrew
Trust Class: Homebrew
Identity: VALID
Policy: RESTRICTED
Restrictions:
    DMA disabled
    Boot disabled
    Supervisor access disabled
```

## Recovery Behavior

Recovery must not depend on Directory Services.

If identity validation fails, the supervisor and Backplane BIOS must still provide enough diagnostic access to determine:

- whether the EEPROM is missing
- whether communication failed
- whether CRC failed
- whether identity was rejected by policy
- whether provisioning is required

## Relationship to Discovery ROM

Identity EEPROM and Discovery ROM are related but distinct.

Identity EEPROM answers:

```text
Who are you?
What board are you?
How were you provisioned?
How much should I initially trust you?
```

Discovery ROM answers:

```text
What resources and services do you expose?
How should software bind to them?
```

A valid Identity EEPROM is required before Discovery ROM information is trusted.

## Relationship to Capability Records

Capability flags in the EEPROM are only a compact summary.

Detailed capabilities belong in Capability Records.

A hardware capability does not imply authorization to use that capability.

## Future Expansion

The mandatory 512-byte block reserves space for v1-compatible extension pointers or future fields.

Future extension blocks may include:

- certificate block
- cryptographic signature
- public key
- asset tags
- calibration data
- manufacturing notes
- service history
- repair history
- signed capability grants

Rev A does not require cryptographic trust.

The format reserves room for it.

## Implementation Targets

This specification should produce:

```text
include/atarix/identity_eeprom.h
tests/eeprom/
src/emulator/discovery/identity_eeprom.*
src/firmware/monitor/eeprom.*
```

## Non-Goals

This specification does not define:

- Discovery ROM binary format
- Capability Record binary format
- Service Directory format
- cryptographic signing policy
- official vendor allocation process

Those are separate specifications.
