# Service ID Registry v1

Status: Draft v1

## Purpose

This document defines the canonical ATARIX Service ID registry.

Service IDs identify software-visible services exposed through the Enumeration Fabric and later bound through Directory Services and Transport mechanisms.

Service IDs are used by:

- Discovery ROM
- Service Directory
- Capability Records
- Backplane BIOS
- Monitor
- Operating system services
- Supervisor and instrumentation tooling

A Service ID identifies a class of service.

A Service ID does not identify a specific instance.

Specific instances are identified through handles and directory records.

## Architectural Rules

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Rings constrain authority.
Directory locates.
Transport operates.
```

Service IDs do not grant authority.

Service IDs do not imply trust.

Service IDs do not imply capability.

## Service ID Ranges

```text
0x0000          Reserved / invalid
0x0001-0x0FFF   Core platform services
0x1000-0x1FFF   Instrumentation services
0x2000-0x2FFF   CPU services
0x3000-0x3FFF   Network services
0x4000-0x4FFF   Storage services
0x5000-0x5FFF   Reserved
0x6000-0x6FFF   Accelerator services
0x7000-0x7FFF   Reserved
0x8000-0xFFFF   Experimental / homebrew services
```

## Core Platform Services

```text
0x0001  SERVICE_DISCOVERY
0x0002  SERVICE_DIRECTORY
0x0003  SERVICE_PROVISIONING
0x0004  SERVICE_VALIDATION
0x0005  SERVICE_MONITOR
0x0006  SERVICE_HEALTH
0x0007  SERVICE_FAULT_LOG
0x0008  SERVICE_EVENT_LOG
0x0009  SERVICE_TIMEBASE
0x000A  SERVICE_MAILBOX
0x000B  SERVICE_DMA
0x000C  SERVICE_INTERRUPT_ROUTING
0x000D  SERVICE_RECOVERY
```

## Instrumentation Services

```text
0x1000  SERVICE_INSTRUMENTATION
0x1001  SERVICE_CAPTURE
0x1002  SERVICE_ANALYSIS
0x1003  SERVICE_TRACE
0x1004  SERVICE_LOGIC_ANALYZER
0x1005  SERVICE_OSCILLOSCOPE
0x1006  SERVICE_PATTERN_GENERATOR
0x1007  SERVICE_PROTOCOL_DECODER
0x1008  SERVICE_COUNTERS
0x1009  SERVICE_TRIGGER_ROUTER
```

## CPU Services

```text
0x2000  SERVICE_CPU_DIAGNOSTICS
0x2001  SERVICE_CPU_HEALTH
0x2002  SERVICE_CPU_TRACE
0x2003  SERVICE_CPU_BOOT_STATUS
0x2004  SERVICE_CPU_VALIDATION
```

## Network Services

```text
0x3000  SERVICE_NETWORK_INTERFACE
0x3001  SERVICE_NETWORK_BOOT
0x3002  SERVICE_PACKET_TRANSPORT
0x3003  SERVICE_TIME_SYNC
```

## Storage Services

```text
0x4000  SERVICE_BLOCK_STORAGE
0x4001  SERVICE_FILE_STORAGE
0x4002  SERVICE_ROM_STORAGE
0x4003  SERVICE_CONFIGURATION_STORAGE
```

## Accelerator Services

```text
0x6000  SERVICE_GPU_COMPUTE
0x6001  SERVICE_AI_ACCELERATION
0x6002  SERVICE_VECTOR_COMPUTE
0x6003  SERVICE_FPGA_COMPUTE
0x6004  SERVICE_CRYPTO_ACCELERATION
```

## Experimental / Homebrew Services

```text
0x8000 - 0xFFFF
```

Reserved for:

- prototype services
- homebrew services
- experimental firmware
- development systems

No central registry assignment is required for this range.

Products intended for general release should migrate to assigned Service IDs.

## Ring Security Notes

Service IDs may be associated with default ownership, visibility, and control rings in future security policy documents.

Examples:

```text
SERVICE_RECOVERY
    Owner:   Ring -2
    Visible: Ring -2 / Ring -1
    Control: Ring -2

SERVICE_PATTERN_GENERATOR
    Owner:   Ring -2 / Ring -1
    Visible: Ring -2 / Ring -1 / Ring 0 by policy
    Control: Ring -2 / Ring -1 unless delegated

SERVICE_CPU_HEALTH
    Owner:   Ring -2 / Ring -1
    Visible: Ring -2 / Ring -1 / Ring 0
    Control: Ring -2 / Ring -1
```

These notes are advisory in this registry.

Actual authorization belongs to capability and ring policy.

## Allocation Policy

Assigned IDs shall not be reused.

Deprecated IDs remain reserved permanently.

Service IDs are allocated by registry update.

## Related Documents

- discovery-rom-format-v1.md
- resource-type-registry-v1.md
- ADR-RING-SECURITY-MODEL.md
- capability-record-format-v1.md
- service-directory-format-v1.md
