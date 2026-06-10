# ATARIX Card Identity EEPROM

## Status

Draft architecture specification.

This document defines the small nonvolatile identity device that should be present on each ATARIX card.

## Purpose

Each ATARIX card should include a small identity EEPROM that allows the supervisor, boot firmware, or fabric controller to identify the card installed in a slot before the main card logic is fully initialized.

The identity EEPROM provides a stable, low-power, early-boot source of information for:

- Card identity.
- Vendor and product identifiers.
- Hardware revision.
- Serial number.
- Board capabilities.
- Power requirements.
- Required firmware or bitstream versions.
- Discovery-record location.
- Manufacturing data.
- Recovery and safe-mode information.

## Historical Influences

The design is inspired by:

- NuBus declaration-ROM style self-description.
- EISA-style configuration records.
- Modern SPD EEPROMs used on memory modules.
- Server FRU inventory EEPROMs.

ATARIX should borrow the self-description idea, not copy any legacy format directly.

## Design Rule

A card should be identifiable before it is trusted.

A card should be identifiable before its main processor, FPGA, CPLD, or device logic is allowed to perform privileged fabric activity.

Attachment does not imply trust.

## Relationship to Discovery Records

The identity EEPROM is not a replacement for the full discovery system.

Instead:

```text
Identity EEPROM
    Small, early, low-level, always readable where practical.

Discovery Record
    Larger, richer, versioned, fabric-visible system description.
```

The EEPROM may contain either:

1. A compact identity record directly, or
2. A pointer to a richer discovery record stored elsewhere, or
3. Both.

Possible richer discovery locations:

```text
On-card ROM
On-card flash
FPGA block RAM
Supervisor-provided table
Fabric-generated record
Boot-updated record cache
```

## Bus Interface

Recommended Rev A interface:

```text
I2C-compatible serial EEPROM
```

Rationale:

- Simple wiring.
- Low pin count.
- Easy supervisor access.
- Similar operational model to SPD EEPROMs.
- Easy read access before full card initialization.

The backplane should provide an identity-management path that can be accessed by the supervisor and/or fabric controller.

## Slot Addressing

The system must be able to distinguish EEPROMs by slot.

Possible approaches:

- Slot-specific I2C mux.
- Slot address pins into the EEPROM address pins.
- Supervisor-controlled per-slot enable.
- Fabric-controlled identity bus arbitration.

Rev A should prefer the simplest electrically reliable scheme.

## Write Policy

The identity EEPROM should contain manufacturing identity that is normally read-only in system operation.

Writable regions may be allowed for:

- Boot-generated cached discovery data.
- Calibration data.
- Board configuration.
- Field-service data.
- Last-known-good configuration pointer.

However, writes must be controlled.

Recommended policy:

```text
Manufacturing region: write-protected after programming.
System region: writable only by supervisor-authorized process.
Recovery region: writable only in explicit maintenance mode.
```

The normal operating system should not freely rewrite identity EEPROM contents.

## Identity Record v1

All multi-byte fields are little-endian and follow:

```text
docs/data-model-and-endianness.md
```

Recommended fixed header:

```text
Offset  Size  Field
+00     4     Magic: "ATID"
+04     1     Format Major Version
+05     1     Format Minor Version
+06     2     Header Length, u16
+08     2     Total Length, u16 or extended form
+0A     2     Header Checksum, u16
+0C     2     Vendor ID, u16
+0E     2     Product ID, u16
+10     2     Device Class, u16
+12     2     Device Subclass, u16
+14     2     Hardware Revision, u16
+16     2     Board Revision, u16
+18     8     Serial Number or Board ID, u64
+20     8     Capability Summary, u64
+28     8     Required Power Summary, u64
+30     8     Discovery Pointer or Resource ID, u64
+38     8     Manufacturing Timestamp, u64
+40     8     Reserved, must be zero
```

Minimum header size:

```text
72 bytes
```

The EEPROM may then contain TLV records.

## Standard TLV Records

Recommended TLV record form:

```text
Offset  Size  Field
+00     1     Type
+01     1     Flags
+02     2     Length, u16
+04     N     Value
```

For longer values, use a TLV type that explicitly contains a u32 or u64 length inside the value.

Recommended TLV types:

```text
$00 End
$01 Human-readable card name
$02 Manufacturer name
$03 Product name
$04 Firmware compatibility
$05 FPGA bitstream compatibility
$06 Power requirements
$07 Clock requirements
$08 Reset requirements
$09 Discovery record location
$0A Required capability classes
$0B Supported safe modes
$0C Diagnostic entry points
$0D Manufacturing data
$0E Field-service data
$0F Calibration data
$10 Boot cache pointer
$80-$FF Vendor-specific
```

## Required Fields

Every card identity EEPROM should provide at minimum:

```text
Magic
Format version
Vendor ID
Product ID
Device class
Hardware revision
Serial number or board ID
Capability summary
Power requirement summary
```

## Capability Summary

The capability summary is not a security grant.

It is only an early-boot hint describing what the card claims to support.

Actual access still requires capability authorization.

Examples:

```text
DMA capable
Mailbox capable
Interrupt capable
Memory service capable
Storage service capable
Accelerator capable
Supervisor visible
Fabric configuration required
```

## Boot Flow

Recommended boot sequence:

```text
1. Supervisor powers identity bus.
2. Supervisor scans slots.
3. Supervisor reads each identity EEPROM.
4. Supervisor validates checksum and version.
5. Supervisor records slot inventory.
6. Fabric controller receives slot identity summary.
7. Boot firmware queries fabric identity table.
8. Full discovery records are read or generated.
9. Capability broker assigns permissions.
10. Devices become usable only after authorization.
```

## Identity Fabric

The identity fabric is the early-boot inventory path that collects identity information from cards.

It may be implemented by:

- Supervisor firmware.
- Fabric-controller logic.
- A dedicated identity bus.
- A combination of supervisor and fabric support.

The identity fabric should expose a stable inventory table to boot firmware and diagnostics.

## Security Considerations

Identity is not authority.

A card may claim capabilities in its EEPROM, but the system must not grant access solely because the EEPROM says so.

Security rules:

1. Treat EEPROM data as untrusted until validated.
2. Verify checksums and versions.
3. Apply policy before granting capabilities.
4. Do not allow unrestricted EEPROM writes.
5. Record identity mismatches in supervisor logs.
6. Quarantine cards with invalid or inconsistent identity data where appropriate.

## Diagnostics

The supervisor and firmware monitor should support commands similar to:

```text
IDENT LIST
IDENT SHOW <slot>
IDENT RAW <slot>
IDENT VERIFY <slot>
IDENT CACHE REFRESH
```

## Relationship to Testing

Testing should include:

- Valid identity EEPROM parsing.
- Invalid checksum handling.
- Unknown version handling.
- Missing EEPROM handling.
- Conflicting discovery-record handling.
- Write-protection checks.
- Slot inventory stability.

See:

```text
docs/testing-strategy.md
```

## Design Principle

The identity EEPROM provides early, minimal, stable identity.

Discovery records provide rich, versioned system description.

Capabilities provide authority.

These must remain separate.