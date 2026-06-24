# Resource Type Registry v1

Status: Draft v1

## Purpose

This document defines the canonical ATARIX Resource Type ID registry.

Resource Type IDs identify classes of resources exposed through the Enumeration Fabric.

Resource Type IDs are used by:

- Discovery ROM
- Enumeration services
- Capability records
- Directory services
- Validation services
- Instrumentation services

A Resource Type ID identifies a class of resource.

A Resource Type ID does not identify a specific instance.

Specific instances are identified through handles.

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

Resource Type IDs do not grant authority.

Resource Type IDs do not imply trust.

Resource Type IDs do not imply capability.

## Resource ID Ranges

```text
0x0000          Reserved / invalid
0x0001-0x0FFF   Core resources
0x1000-0x1FFF   Instrumentation resources
0x2000-0x2FFF   CPU resources
0x3000-0x3FFF   Network resources
0x4000-0x4FFF   Storage resources
0x5000-0x5FFF   Reserved
0x6000-0x6FFF   Accelerator resources
0x7000-0x7FFF   Reserved
0x8000-0xFFFF   Experimental / homebrew resources
```

## Core Resources

```text
0x0001  RESOURCE_MAILBOX
0x0002  RESOURCE_DMA
0x0003  RESOURCE_TIMER
0x0004  RESOURCE_INTERRUPT_CONTROLLER
0x0005  RESOURCE_GPIO
0x0006  RESOURCE_UART
0x0007  RESOURCE_FABRIC_MEMORY
0x0008  RESOURCE_SHARED_MEMORY
0x0009  RESOURCE_EVENT_QUEUE

0x000A  RESOURCE_ENUMERATION_FABRIC
0x000B  RESOURCE_MANAGEMENT_FABRIC
0x000C  RESOURCE_TRANSPORT_FABRIC

0x0010  RESOURCE_SUPERVISOR
0x0011  RESOURCE_HEALTH_MONITOR
0x0012  RESOURCE_PROVISIONING_ENGINE
0x0013  RESOURCE_VALIDATION_ENGINE
```

## Instrumentation Resources

```text
0x1000  RESOURCE_LOGIC_ANALYZER
0x1001  RESOURCE_OSCILLOSCOPE
0x1002  RESOURCE_DSP
0x1003  RESOURCE_PATTERN_GENERATOR
0x1004  RESOURCE_PROTOCOL_DECODER
0x1005  RESOURCE_COUNTER_BANK
0x1006  RESOURCE_TRIGGER_ROUTER
0x1007  RESOURCE_TRACE_BUFFER
```

## CPU Resources

```text
0x2000  RESOURCE_CPU
0x2001  RESOURCE_CPU_DIAGNOSTICS
0x2002  RESOURCE_CPU_TRACE
0x2003  RESOURCE_CPU_HEALTH
0x2004  RESOURCE_CPU_BOOT_STATUS
```

## Network Resources

```text
0x3000  RESOURCE_NETWORK_INTERFACE
0x3001  RESOURCE_PACKET_ENGINE
0x3002  RESOURCE_NETWORK_BOOT
```

## Storage Resources

```text
0x4000  RESOURCE_BLOCK_STORAGE
0x4001  RESOURCE_FILE_STORAGE
0x4002  RESOURCE_ROM_STORAGE
0x4003  RESOURCE_CONFIGURATION_STORAGE
```

## Accelerator Resources

```text
0x6000  RESOURCE_GPU
0x6001  RESOURCE_AI_ACCELERATOR
0x6002  RESOURCE_VECTOR_ENGINE
0x6003  RESOURCE_FPGA_COMPUTE
0x6004  RESOURCE_CRYPTO_ENGINE
```

## Experimental / Homebrew Resources

```text
0x8000 - 0xFFFF
```

Reserved for:

- prototype cards
- homebrew hardware
- experimental firmware
- development systems

No central registry assignment is required for this range.

Products intended for general release should migrate to assigned Resource Type IDs.

## Ring Security Notes

Resource Type IDs may be associated with default ownership, visibility, and control rings in future security policy documents.

Examples:

```text
RESOURCE_SUPERVISOR
    Owner:   Ring -2
    Visible: Ring -2 / Ring -1
    Control: Ring -2

RESOURCE_PATTERN_GENERATOR
    Owner:   Ring -2 / Ring -1
    Visible: Ring -2 / Ring -1 / Ring 0 by policy
    Control: Ring -2 / Ring -1 unless delegated
```

These notes are advisory in this registry.

Actual authorization belongs to capability and ring policy.

## Allocation Policy

Assigned IDs shall not be reused.

Deprecated IDs remain reserved permanently.

Resource Type IDs are allocated by registry update.

## Related Documents

- discovery-rom-format-v1.md
- service-id-registry-v1.md
- ADR-RING-SECURITY-MODEL.md
- capability-record-format-v1.md
- card-health-model-v1.md
