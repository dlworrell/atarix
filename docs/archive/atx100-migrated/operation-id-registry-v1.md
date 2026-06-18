# Operation ID Registry v1

Status: Draft v1

## Purpose

This document defines the canonical ATARIX Operation ID registry.

Operation IDs identify actions that may be authorized by Capability Records.

Operation IDs are used by:

- Capability Records
- Supervisor policy
- Ring security policy
- Service Directory binding
- Validation tools
- Monitor commands

An Operation ID identifies an action.

An Operation ID does not grant authority.

## Architectural Rules

```text
Identity identifies.
Discovery describes.
Trust evaluates.
Capabilities authorize.
Rings constrain authority.
Directory locates.
Transport operates.
```

## Operation ID Ranges

```text
0x00000000              Reserved / invalid
0x00000001-0x00000FFF   Core operations
0x00001000-0x00001FFF   Instrumentation operations
0x00002000-0x00002FFF   CPU operations
0x00003000-0x00003FFF   Network operations
0x00004000-0x00004FFF   Storage operations
0x80000000-0xFFFFFFFF   Experimental / homebrew operations
```

## Core Operations

```text
0x00000001  OP_READ
0x00000002  OP_WRITE
0x00000003  OP_CONTROL
0x00000004  OP_RESET
0x00000005  OP_CAPTURE
0x00000006  OP_STIMULATE
0x00000007  OP_BOOT
0x00000008  OP_RECOVER
0x00000009  OP_PROVISION
0x0000000A  OP_ENUMERATE
0x0000000B  OP_VALIDATE
0x0000000C  OP_MONITOR
0x0000000D  OP_BIND
0x0000000E  OP_DELEGATE
0x0000000F  OP_REVOKE
```

## Instrumentation Operations

```text
0x00001000  OP_TRACE_START
0x00001001  OP_TRACE_STOP
0x00001002  OP_TRACE_EXPORT
0x00001003  OP_TRIGGER_ARM
0x00001004  OP_TRIGGER_FIRE
0x00001005  OP_PATTERN_LOAD
0x00001006  OP_PATTERN_START
0x00001007  OP_PATTERN_STOP
```

## CPU Operations

```text
0x00002000  OP_CPU_READ_HEALTH
0x00002001  OP_CPU_READ_BOOT_STATUS
0x00002002  OP_CPU_HOLD_RESET
0x00002003  OP_CPU_RELEASE_RESET
0x00002004  OP_CPU_CAPTURE_TRACE
0x00002005  OP_CPU_VALIDATE
```

## Security Notes

Operation IDs are interpreted through Capability Records and ring policy.

Examples:

```text
OP_READ
    may be safe for lower rings depending on target

OP_STIMULATE
    generally requires Ring -2 or Ring -1

OP_PROVISION
    generally requires Ring -2

OP_BOOT
    requires explicit boot authority
```

## Allocation Policy

Assigned IDs shall not be reused.

Deprecated IDs remain reserved permanently.

Operation IDs are allocated by registry update.

## Related Documents

- capability-record-format-v1.md
- ADR-RING-SECURITY-MODEL.md
- service-directory-format-v1.md
