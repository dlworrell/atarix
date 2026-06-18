# Service Directory Format v1

Status: Draft v1

## Purpose

The Service Directory maps discovered and authorized services to bindable runtime entries.

The Service Directory answers:

```text
Where is it?
Who provides it?
Which handle implements it?
How do I bind to it?
```

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

The Service Directory does not:

- identify hardware
- establish trust
- grant capabilities
- override ring policy
- replace supervisor safety authority

## Directory Header

```text
Offset  Size  Field
------  ----  ----------------------
0x00    4     Magic
0x04    2     Version Major
0x06    2     Version Minor
0x08    4     Total Length
0x0C    4     Entry Count
0x10    4     Flags
0x14    4     Header CRC32
0x18    4     Directory CRC32
0x1C    4     Reserved
```

Magic:

```text
ATDR
```

Meaning:

```text
ATARIX Directory
```

Header size:

```text
32 bytes
```

## Directory Entry

All entries are 8-byte aligned.

```text
Offset  Size  Field
------  ----  ----------------------
0x00    8     Service Handle
0x08    8     Provider Handle
0x10    4     Service ID
0x14    4     Provider ID
0x18    4     Flags
0x1C    4     Binding Type
0x20    8     Binding Handle
0x28    8     Required Capability Handle
0x30    4     Minimum Ring
0x34    4     Reserved
```

Entry size:

```text
56 bytes
```

## Binding Type

Initial binding types:

```text
0x00000000  Invalid
0x00000001  Mailbox
0x00000002  Transport handle
0x00000003  Shared memory
0x00000004  Supervisor call
0x00000005  Fabric service
0x00000006  Network endpoint
0x00000007  Storage endpoint
```

## Flags

Initial flags:

```text
Bit 0   Available
Bit 1   Authorized
Bit 2   Restricted
Bit 3   Recovery only
Bit 4   Development only
Bit 5   Hotplug capable
Bit 6   Instrumented
Bits 7-31 reserved
```

## Directory Construction

Recommended construction sequence:

```text
Identity validation
    ↓
Discovery enumeration
    ↓
Trust evaluation
    ↓
Capability evaluation
    ↓
Ring policy filtering
    ↓
Directory construction
```

The Directory should contain the authorized platform view appropriate to the requesting ring.

## Ring Filtering

Different rings may receive different directory views.

Example:

```text
Ring -2
    Full supervisor view

Ring -1
    Platform firmware view

Ring 0
    Kernel-authorized services

Ring 3
    Application-safe services only
```

## Recovery Rule

Recovery must not depend on Directory Services.

The supervisor must retain direct diagnostic access when the directory is unavailable, corrupted, or incomplete.

## Security Rules

Directory entries do not grant authority.

Directory entries describe authorized binding results after capability and ring policy have already been evaluated.

## Related Documents

- discovery-rom-format-v1.md
- service-id-registry-v1.md
- capability-record-format-v1.md
- trust-model-v1.md
- ADR-RING-SECURITY-MODEL.md
