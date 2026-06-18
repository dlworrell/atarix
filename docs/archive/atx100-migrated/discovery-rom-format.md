# ATARIX Discovery ROM Format

## Status

Draft specification for the ATARIX hybrid discovery format.

This design uses a fixed header followed by typed length-value records. It is intended to be simple enough for W65C816 monitor firmware to parse while remaining extensible enough for later CPU cards, FPGA services, backplane modules, and accelerators.

## Design Choice

ATARIX uses a hybrid discovery model:

- Fixed header for fast identification.
- Vendor, device, class, and revision fields for stable driver matching.
- TLV records for capabilities, memory windows, interrupts, DMA, mailboxes, human-readable names, and future extensions.

This is inspired by NuBus-style self-description, PCI-style identity fields, and device-tree-style extensibility, but it is deliberately simpler than any of those systems.

## Goals

1. Easy parsing from ROM monitor firmware.
2. Stable driver binding.
3. Extensible capability declaration.
4. Human-readable diagnostics.
5. Compatibility with future capability-based security.
6. No mandatory dynamic allocation during early boot.

## Byte Order

All multi-byte fields are little-endian.

## Alignment

Discovery records should be aligned to 2-byte boundaries.

Padding bytes must be zero.

## Fixed Header

```text
Offset  Size  Field
+00     4     Magic: "ATDX"
+04     1     Format Major Version
+05     1     Format Minor Version
+06     2     Header Length
+08     2     Total Length
+0A     2     Header Checksum
+0C     2     Vendor ID
+0E     2     Device ID
+10     2     Device Class
+12     2     Device Subclass
+14     2     Device Revision
+16     2     Flags
+18     4     TLV Offset
+1C     4     Reserved
```

Minimum header length:

```text
32 bytes
```

## Header Fields

### Magic

The magic field must contain:

```text
ATDX
```

Meaning:

```text
ATARIX Discovery
```

### Format Version

Major version changes indicate incompatible format changes.

Minor version changes indicate backward-compatible extensions.

### Header Length

Length of the fixed header in bytes.

### Total Length

Total discovery structure length in bytes, including header, TLV records, and padding.

### Header Checksum

A simple 16-bit checksum over the fixed header with the checksum field treated as zero.

This is not a security feature. It is only a corruption-detection feature.

### Vendor ID

Vendor or project identifier.

Reserved values:

```text
$0000 Invalid
$0001 ATARIX Project
$FFFF Experimental / Local
```

### Device ID

Vendor-defined device identifier.

### Device Class

Primary device class.

```text
$0000 Unknown
$0001 CPU Card
$0002 Memory Device
$0003 Serial Device
$0004 Network Device
$0005 Storage Device
$0006 FPGA Fabric Function
$0007 Supervisor Controller
$0008 Timer Device
$0009 DMA Engine
$000A Accelerator
$000B Backplane Service
$8000-$FFFF Experimental
```

### Device Subclass

Class-specific subtype.

### Device Revision

Hardware or firmware revision.

### Flags

```text
Bit 0 Boot Critical
Bit 1 Requires Driver
Bit 2 Has Interrupts
Bit 3 DMA Capable
Bit 4 Mailbox Capable
Bit 5 Capability Restricted
Bit 6 Hot Reset Safe
Bit 7 Supervisor Visible
Bits 8-15 Reserved
```

## TLV Record Format

Each TLV record begins with:

```text
Offset  Size  Field
+00     1     Type
+01     1     Length
+02     N     Value
```

Length is the size of the value field only.

A type of `$00` marks the end of records.

## Standard TLV Types

```text
$00 End
$01 Human-Readable Name
$02 Description
$03 Memory Window
$04 I/O Register Window
$05 Interrupt Source
$06 DMA Capability
$07 Mailbox Endpoint
$08 Firmware Version
$09 Driver Binding
$0A Capability Requirement
$0B Power Requirement
$0C Clock Requirement
$0D Reset Behavior
$0E Slot Information
$0F Diagnostic Entry Point
$10 Vendor-Specific Data
$80-$FF Experimental
```

## Name Record

Type:

```text
$01
```

Value:

```text
ASCII string, not necessarily null-terminated
```

Example:

```text
ATARIX Rev B Supervisor
```

## Memory Window Record

Type:

```text
$03
```

Value:

```text
Offset  Size  Field
+00     3     Base Address, 24-bit
+03     3     Length, 24-bit
+06     1     Attributes
```

Attributes:

```text
Bit 0 Readable
Bit 1 Writable
Bit 2 Executable
Bit 3 DMA Visible
Bit 4 Cacheable, future use
Bits 5-7 Reserved
```

## I/O Register Window Record

Type:

```text
$04
```

Value:

```text
Offset  Size  Field
+00     3     Base Address, 24-bit
+03     2     Length, 16-bit
+05     1     Access Width
```

Access width:

```text
$01 8-bit
$02 16-bit
$03 Mixed
```

## Interrupt Source Record

Type:

```text
$05
```

Value:

```text
Offset  Size  Field
+00     1     Interrupt Source ID
+01     1     Default Priority
+02     1     Flags
```

Flags:

```text
Bit 0 Level Triggered
Bit 1 Edge Triggered
Bit 2 Maskable
Bit 3 Wake Capable
```

## DMA Capability Record

Type:

```text
$06
```

Value:

```text
Offset  Size  Field
+00     1     DMA Channels
+01     1     Address Width
+02     1     Flags
```

Address width is measured in bits.

Flags:

```text
Bit 0 Read From Memory
Bit 1 Write To Memory
Bit 2 Scatter/Gather Capable
Bit 3 Requires Capability Grant
```

## Mailbox Endpoint Record

Type:

```text
$07
```

Value:

```text
Offset  Size  Field
+00     1     Endpoint ID
+01     1     Max Message Size Low
+02     1     Max Message Size High
+03     1     Flags
```

Flags:

```text
Bit 0 Interrupt On Receive
Bit 1 Polling Supported
Bit 2 Responses Supported
Bit 3 Capability Required
```

## Driver Binding Record

Type:

```text
$09
```

Value:

```text
ASCII string
```

Example:

```text
atarix,w5500
```

## Capability Requirement Record

Type:

```text
$0A
```

Value:

```text
Offset  Size  Field
+00     2     Required Capability Class
+02     2     Required Capability Flags
```

This record allows a device to declare that it requires mediated access before normal use.

## End Record

Type:

```text
$00
```

Length:

```text
$00
```

## Parser Requirements

A minimal parser must:

1. Verify magic.
2. Check major version.
3. Verify header length.
4. Verify total length is reasonable.
5. Optionally verify header checksum.
6. Iterate TLV records until end marker or total length.
7. Ignore unknown TLV types.

## Monitor Commands

Future ROM monitor commands should support:

```text
DISCOVER
DISCOVER LIST
DISCOVER SHOW <device>
DISCOVER RAW <device>
```

## Open Questions

- Final checksum algorithm.
- Whether total length should be 16-bit or 24-bit.
- Whether device identity should include serial number fields.
- Whether discovery records live in ROM, EEPROM, FPGA block RAM, or supervisor-provided memory.
- Whether human-readable strings should be ASCII-only or UTF-8.

## Versioning Rule

Version 1 discovery structures must remain parseable by future firmware.

Unknown TLV records must be ignored unless marked as mandatory by a future extension.