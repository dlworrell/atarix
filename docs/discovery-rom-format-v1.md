# Discovery ROM Format v1

Status: Draft v1

## Purpose

The Discovery ROM provides a platform-independent description of resources, services, instrumentation endpoints, providers, and dependencies offered by a card or subsystem.

The Discovery ROM answers:

```text
What do you provide?
How do clients bind to it?
What does it depend upon?
```

The Discovery ROM does not establish trust, grant authority, or assign permissions.

## Architectural Principles

```text
Discovery describes.
Discovery does not authorize.
Discovery does not establish trust.
Discovery does not grant capability.
```

```text
Address != Handle.
CPU Width != Fabric Width.
```

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Transport operates.
```

## Relationship to Identity EEPROM

Identity EEPROM provides hardware identity.

Example:

```text
Vendor: ATARIX
Board: CPU Card Rev A
Serial: 00000123
```

Discovery ROM provides firmware, resource, and service description.

Example:

```text
Provider: W65C816 Runtime
Version: 1.0
Services:
    CPU Diagnostics
    Mailbox
    Instrumentation
```

Firmware updates may change Discovery ROM contents without changing hardware identity.

## Design Goals

The Discovery ROM shall be:

```text
Architecture-neutral
Versioned
Self-describing
Checksummed
Forward-compatible
CPU-independent
```

## Endianness and Alignment

All numeric fields are little-endian.

All records shall begin on an 8-byte boundary.

Record lengths include the common record header and any padding required to align the next record.

## Discovery Header

All Discovery ROM images begin with a 32-byte Discovery Header.

### Header Layout

```text
Offset  Size  Field
------  ----  ----------------------
0x00    4     Magic
0x04    2     Version Major
0x06    2     Version Minor
0x08    4     Total Length
0x0C    4     Record Count
0x10    4     Provider ID
0x14    4     Flags
0x18    4     Header CRC32
0x1C    4     Image CRC32
```

### Magic Value

The magic field shall contain:

```text
ATDX
```

ASCII bytes:

```text
0x41 0x54 0x44 0x58
```

Meaning:

```text
ATARIX Discovery
```

### Versioning

```text
Version Major
    Breaking format changes

Version Minor
    Backward-compatible additions
```

Initial version:

```text
Major = 1
Minor = 0
```

### Total Length

Total Length is the byte length of the entire Discovery ROM image, including the Discovery Header, all records, padding, and the END record.

### Record Count

Record Count is the number of records following the Discovery Header, including the END record.

### Provider ID

Provider ID is a 32-bit registry-assigned identifier for the firmware, runtime, or service provider that generated the Discovery ROM.

Provider ID is not derived from the Identity EEPROM.

Identity EEPROM identifies hardware.

Provider ID identifies software, firmware, or runtime providers.

Initial examples:

```text
0x00000001  Supervisor Runtime
0x00000002  Fabric Runtime
0x00000003  W65C816 Runtime
0x00000004  Validation Runtime
0x00000005  Instrumentation Runtime
```

A formal provider registry may be defined later.

### Flags

Initial Discovery Header flags:

```text
Bit 0   Boot required
Bit 1   Runtime provider
Bit 2   Instrumented
Bit 3   CPU resource provider
Bit 4   Hotplug capable
Bit 5   Experimental
Bits 6-31 reserved
```

### Header CRC32

Header CRC32 covers bytes 0x00 through 0x1F of the Discovery Header with the Header CRC32 field treated as zero during calculation.

### Image CRC32

Image CRC32 covers the entire Discovery ROM image with the Image CRC32 field treated as zero during calculation.

CRC algorithm:

```text
CRC-32/ISO-HDLC
```

unless superseded by a later revision.

## Handle Format

Discovery records shall use 64-bit handles.

```text
typedef uint64_t atarix_discovery_handle_t;
```

Handle Encoding v1:

```text
63........48  Provider ID low 16 bits
47........32  Resource or Service Type
31.........0  Instance ID
```

A handle is not an address.

A handle must not be interpreted as a CPU-local address, Fabric address, physical address, IRQ number, or DMA channel.

## Common Record Header

All records begin with an 8-byte common header.

```c
struct discovery_record_header {
    uint16_t type;
    uint16_t version;
    uint32_t length;
};
```

Unknown record types shall be skipped using `length`.

## Record Type Registry

```text
0x0000  RESERVED_RECORD
0x0001  RESOURCE_RECORD
0x0002  SERVICE_RECORD
0x0003  PROVIDER_RECORD
0x0004  DEPENDENCY_RECORD
0x0005  INSTRUMENTATION_RECORD
0xFFFF  END_RECORD
```

Experimental and vendor-specific record range:

```text
0x8000 - 0xFFFE
```

## Resource Records

Resource Records describe platform resources.

Examples:

```text
RESOURCE_MAILBOX
RESOURCE_DMA
RESOURCE_TIMER
RESOURCE_GPIO
RESOURCE_UART
RESOURCE_INTERRUPT_CONTROLLER
RESOURCE_NETWORK
RESOURCE_STORAGE
RESOURCE_FABRIC_MEMORY
RESOURCE_LOGIC_ANALYZER
RESOURCE_OSCILLOSCOPE
RESOURCE_DSP
RESOURCE_PATTERN_GENERATOR
RESOURCE_PROTOCOL_DECODER
RESOURCE_COUNTER_BANK
RESOURCE_TRIGGER_ROUTER
```

### Resource Record Payload

```text
Offset  Size  Field
------  ----  ----------------------
0x00    2     Resource Type ID
0x02    2     Resource Version
0x04    4     Resource Flags
0x08    8     Resource Handle
0x10    4     Provider ID
0x14    4     Instance ID
```

Total payload size:

```text
24 bytes
```

Total record size including common header:

```text
32 bytes
```

## Service Records

Service Records describe consumable platform services.

Examples:

```text
SERVICE_DIRECTORY
SERVICE_DISCOVERY
SERVICE_PROVISIONING
SERVICE_VALIDATION
SERVICE_MONITOR
SERVICE_NETWORK_BOOT
SERVICE_INSTRUMENTATION
SERVICE_CAPTURE
SERVICE_ANALYSIS
SERVICE_CPU_DIAGNOSTICS
```

### Service Record Payload

```text
Offset  Size  Field
------  ----  ----------------------
0x00    2     Service ID
0x02    2     Service Version
0x04    4     Service Flags
0x08    8     Service Handle
0x10    4     Provider ID
0x14    4     Instance ID
```

Total payload size:

```text
24 bytes
```

Total record size including common header:

```text
32 bytes
```

## Provider Records

Provider Records describe software, firmware, or runtime providers.

Examples:

```text
Supervisor Runtime
Fabric Runtime
W65C816 Runtime
Validation Runtime
Instrumentation Runtime
```

### Provider Record Payload

```text
Offset  Size  Field
------  ----  ----------------------
0x00    4     Provider ID
0x04    4     Provider Version
0x08    4     Provider Flags
0x0C    4     Name Length
0x10    N     Provider Name, UTF-8, padded to 8-byte boundary
```

## Dependency Records

Dependency Records describe relationships between resources and services.

Example:

```text
CPU Diagnostics
    depends on
Mailbox
```

### Dependency Record Payload

```text
Offset  Size  Field
------  ----  ----------------------
0x00    8     Consumer Handle
0x08    8     Provider Handle
0x10    2     Dependency Type
0x12    2     Dependency Version
0x14    4     Dependency Flags
```

Total payload size:

```text
24 bytes
```

Total record size including common header:

```text
32 bytes
```

## Instrumentation Records

Instrumentation Records advertise observability and testbench services.

Examples:

```text
Logic Analyzer
Protocol Decoder
Pattern Generator
Counter Bank
DSP Analysis
Trigger Router
```

### Instrumentation Record Payload

```text
Offset  Size  Field
------  ----  ----------------------
0x00    2     Instrumentation Type
0x02    2     Instrumentation Version
0x04    4     Instrumentation Flags
0x08    8     Instrumentation Handle
0x10    8     Observed Handle
```

Total payload size:

```text
24 bytes
```

Total record size including common header:

```text
32 bytes
```

## END Record

The END record terminates the record stream.

It uses the common record header with:

```text
Type    = 0xFFFF
Version = 0x0001
Length  = 8
```

## Forbidden Contents

Discovery records shall not expose:

```text
Physical addresses
CPU-local addresses
IRQ numbers
DMA channels
Board-specific implementation details
```

The platform binding layer resolves handles into implementation-specific resources.

## Enumeration Flow

Recommended enumeration sequence:

```text
Power On
    ↓
Supervisor Validation
    ↓
Identity Validation
    ↓
Discovery Header Validation
    ↓
Record Enumeration
    ↓
Dependency Validation
    ↓
Capability Evaluation
    ↓
Service Binding
    ↓
Transport Activation
```

## Error Handling

The enumerator shall:

```text
Reject invalid CRC
Reject invalid length
Reject invalid version
Skip unknown record types where possible
Continue enumeration where practical
Avoid trusting discovery data until identity validation succeeds
```

## Future Extensions

Future versions may add:

```text
Capability references
Health model references
Security metadata
Network service metadata
Hotplug metadata
Signed discovery images
```

## Related Documents

- identity-eeprom-v1.md
- cpu-observability-contract-v1.md
- supervisor-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md
- ADR-THREE-FABRIC-ARCHITECTURE.md
- ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md
