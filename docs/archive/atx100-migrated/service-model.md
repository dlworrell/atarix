# ATARIX Service Model

Status: Baseline

## Purpose

This document defines the ATARIX service model.

ATARIX is service-addressed, not address-addressed. Applications and higher-level software do not directly access hardware. They access services. Services operate on resources. Resources are implemented by concrete hardware, firmware, simulation, or future node implementations.

## Required Protected-Access Path

All protected access follows this path:

```text
Application
    -> Service
    -> Capability Policy
    -> Resource
    -> Implementation
```

This means:

```text
Applications request service operations.
Services validate capabilities.
Capabilities authorize resource access.
Resources hide implementation details.
Implementations touch hardware or simulated hardware.
```

## Forbidden Access Path

The following pattern is forbidden as a public software contract:

```text
Application
    -> Physical Address
    -> Register
    -> Hardware
```

Examples of forbidden direct access:

```c
*(volatile unsigned int *)0xF0001000u = value;
```

```c
write_register(0xD123u, value);
```

unless such code exists inside a resource implementation layer that is already protected by service and capability policy.

## Visibility Is Not Authority

Discovery grants visibility.

Directory resolves and describes.

Capabilities authorize.

Knowing any of the following does not grant access:

```text
Service name
Handle
Physical address
Register offset
Slot number
Mailbox endpoint
Discovery record
Directory entry
```

## Example: Storage

The caller sees:

```text
Storage Service
```

The protected path is:

```text
Application
    -> Storage Service
    -> Storage Capability
    -> Block Resource
    -> Storage Implementation
```

The implementation may be:

```text
CF card
SD card
SATA device
Network block service
Simulator file
Future Fabric storage card
```

The caller must not depend on the implementation.

## Example: Display

The caller sees:

```text
Display Service
```

The protected path is:

```text
Application
    -> Display Service
    -> Display Capability
    -> Framebuffer Resource
    -> Display Implementation
```

The implementation may be:

```text
FPGA framebuffer
Local video card
Remote display node
Simulator framebuffer
```

## Example: GPIO

The caller sees:

```text
GPIO Service
```

The protected path is:

```text
Application
    -> GPIO Service
    -> GPIO Capability
    -> GPIO Resource
    -> Pin Controller Implementation
```

The caller does not directly manipulate pin registers.

## Simulator Rule

The simulator follows the same model as hardware.

Simulator code must not introduce privileged shortcuts that would be invalid on real hardware.

A simulated resource is still a resource. It is not permission to bypass services or capabilities.

## Service Contracts

A service contract should define:

```text
Service name
Operation identifiers
Request format
Response format
Required capability
Failure statuses
Versioning rules
Observability requirements
```

## Security Requirements

All protected services must:

```text
Deny by default.
Validate capability before access.
Reject malformed requests.
Report explicit failures.
Avoid exposing implementation addresses.
Avoid treating handles as authority.
```

## Result

The ATARIX service model enforces abstraction and security.

Hardware may change. Simulators may replace hardware. Future nodes may provide services. The caller still uses the same service-oriented access path.
