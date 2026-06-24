# Capability Record Format v1

Status: Draft v1

## Purpose

Capability Records define authorized operations against resources and services discovered through the Enumeration Fabric.

Discovery describes what exists.

Capabilities authorize what may be done.

Rings constrain who may exercise that authority.

## Architectural Chain

```text
Identity
    identifies

Discovery
    describes

Trust
    evaluates

Capabilities
    authorize

Rings
    constrain

Directory
    locates

Transport
    operates
```

## Non-Goals

Capability Records do not:

- identify hardware
- enumerate resources
- define physical addresses
- define IRQ numbers
- define DMA channels
- replace supervisor safety policy
- override Ring -2 authority

## Handle Binding

Capabilities target handles produced by Discovery and later resolved by Directory Services.

A capability target is a handle, not an address.

## Capability Record Layout

All numeric fields are little-endian.

All records are 8-byte aligned.

```text
Offset  Size  Field
------  ----  ----------------------
0x00    4     Capability ID
0x04    4     Operation ID
0x08    8     Target Handle
0x10    2     Minimum Ring
0x12    2     Maximum Ring
0x14    4     Flags
0x18    8     Delegation Mask
0x20    8     Expiration / Lifetime
0x28    8     Issuer Handle
0x30    4     Trust Requirement
0x34    4     Reserved
```

Total size:

```text
56 bytes
```

## Capability ID

Capability ID identifies the capability class.

Examples:

```text
READ_HEALTH
READ_COUNTERS
CAPTURE_TRACE
CONTROL_MAILBOX
PERFORM_DMA
BOOT_FROM_RESOURCE
ENTER_RECOVERY
DRIVE_PATTERN_GENERATOR
```

A formal capability ID registry may be defined later.

## Operation ID

Operation ID identifies the specific operation authorized by this record.

Examples:

```text
READ
WRITE
CONTROL
RESET
CAPTURE
STIMULATE
BOOT
RECOVER
PROVISION
```

## Target Handle

Target Handle identifies the resource or service to which the capability applies.

The handle uses the Discovery ROM handle model.

## Minimum Ring and Maximum Ring

Rings constrain who may exercise the capability.

Initial rings:

```text
-2  Hardware Safety / Supervisor Authority
-1  Fabric / Platform Firmware Authority
 0  Kernel / Executive Authority
 1  Driver / Service Authority
 2  User Service Authority
 3  Application Authority
```

Encoding recommendation:

```text
Ring -2 = 0xFFFE
Ring -1 = 0xFFFF
Ring  0 = 0x0000
Ring  1 = 0x0001
Ring  2 = 0x0002
Ring  3 = 0x0003
```

## Flags

Initial flags:

```text
Bit 0   Read allowed
Bit 1   Write allowed
Bit 2   Control allowed
Bit 3   Delegation allowed
Bit 4   Revocation supported
Bit 5   Time limited
Bit 6   Development only
Bit 7   Recovery only
Bit 8   Destructive operation
Bit 9   Requires supervisor approval
Bits 10-31 reserved
```

## Delegation Mask

Delegation Mask describes which lower rings, if any, may receive delegated access.

A zero mask means no delegation.

## Expiration / Lifetime

Expiration may represent:

- zero for no expiration
- monotonic deadline
- policy-defined lifetime
- provisioning epoch

Exact interpretation is policy-defined.

## Issuer Handle

Issuer Handle identifies the provider, supervisor, firmware, or authority that issued the capability.

Issuer authority must be validated by trust and ring policy.

## Trust Requirement

Trust Requirement indicates the minimum trust state required before the capability may be accepted.

Initial values:

```text
0  UNTRUSTED
1  IDENTIFIED
2  VALIDATED
3  TRUSTED
4  PRIVILEGED
5  RESERVED
6  RESERVED
7  REVOKED
```

A capability requiring TRUSTED shall not be accepted from a merely IDENTIFIED entity.

A REVOKED entity shall not grant usable capabilities.

## Security Rules

```text
Discovery describes.
Capabilities authorize.
Rings constrain authority.
Supervisor safety authority overrides capability grants.
```

A valid capability record is not sufficient by itself.

The platform must also evaluate:

- identity
- discovery validity
- trust state
- ring authority
- supervisor policy
- recovery state
- revocation state

## Homebrew Default Policy

Homebrew cards may advertise capabilities, but default platform policy should reject or restrict dangerous operations.

Default denials:

```text
DMA
Boot authority
Recovery authority
Supervisor authority
Memory-controller authority
Privileged instrumentation stimulus
```

## Related Documents

- discovery-rom-format-v1.md
- trust-model-v1.md
- ADR-RING-SECURITY-MODEL.md
- resource-type-registry-v1.md
- service-id-registry-v1.md
- service-directory-format-v1.md
