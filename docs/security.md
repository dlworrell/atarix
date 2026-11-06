<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:365f3bc9-7a8d-4edc-add1-82db51d7a9fd
-->

# ATARIX Security Architecture

## Status

Draft security architecture specification.

This document defines the ATARIX security model, including privilege rings, capability-mediated access, DMA controls, device trust boundaries, accelerator isolation, supervisor recovery, and future multiprocessor considerations.

## Principle

Devices are not trusted by default.

Security is designed into the architecture rather than added later.

ATARIX uses two complementary concepts:

```text
Privilege Rings
    define execution authority.

Capabilities
    define resource authority.
```

A privilege ring answers:

```text
What kind of code is this?
```

A capability answers:

```text
What specific resource may this code or device access?
```

Both are required for a complete security model.

## Privilege Ring Model

ATARIX defines four operating-system privilege rings, 0 through 3. The
canonical definition of every ring, including the platform rings −2 and −1
and the quarantine state, is
[ADR-RING-SECURITY-MODEL](ADR-RING-SECURITY-MODEL.md). This section
summarizes the operating-system rings; if the two ever disagree, the ADR is
correct and this section is the bug.

The rings are inspired by conventional Ring 0 through Ring 3 operating-system
models, adapted to the W65C816 and FPGA-fabric environment. The W65C816 does
not provide hardware rings, so rings are initially a software and
fabric-enforced discipline: the fabric assigns each requester's ring from its
slot, and every mailbox request is checked against it.

## Ring 0 - Kernel and Core Authority

Ring 0 is the most privileged operating-system ring.

Responsibilities:

- Kernel execution.
- Memory-management policy.
- Capability enforcement and brokering.
- Device-driver authority assignment.
- Interrupt-core management.
- DMA grant creation.
- Fabric configuration policy.

Ring 0 may request privileged fabric operations, but it still uses explicit
capability records rather than implicit trust. Ring 0 does not replace the
supervisor management plane.

## Ring 1 - Drivers and Hardware-Facing Services

Ring 1 holds device drivers and the privileged services that operate
hardware.

Examples:

- Trusted block-device and storage drivers.
- Specific hardware drivers.
- Accelerator and display services, such as the display card's GPU compute
  service.
- Console and memory-service managers.
- Debug agents.

Ring 1 components receive narrow capabilities tied to specific devices,
buffers, mailboxes and interrupts, not unrestricted access. They should be
restartable where practical, and a Ring 1 failure should not normally crash
the whole machine.

## Ring 2 - User Services

Ring 2 holds user-space services that use other services but do not operate
hardware.

Examples:

- Filesystem service.
- Network service and protocol workers.
- Desktop and user-session services.
- Application frameworks.

Ring 2 components operate with explicit service capabilities and no direct
hardware authority.

## Ring 3 - User Programs and Untrusted Code

Ring 3 contains ordinary user programs and experimental code.

Ring 3 code should not directly access:

- Raw device registers.
- DMA engines.
- Fabric configuration registers.
- Supervisor controls.
- Privileged interrupt routing.
- Unmediated memory windows.

Ring 3 code must request services through system calls, mailboxes, or mediated service APIs.

## Supervisor Plane - Outside the Ring Model

The supervisor controller is outside the normal Ring 0 through Ring 3
execution model. The platform model numbers it Ring −2, with the fabric at
Ring −1 (see the ADR).

The supervisor is a management and recovery authority, not an application execution ring.

Responsibilities:

- RTC.
- Watchdogs.
- Reset control.
- Fault logging.
- Power-good monitoring.
- Slot quarantine.
- Recovery-mode entry.
- Golden ROM or golden bitstream selection.

The supervisor should not silently perform normal data-plane work.

The supervisor may reset or quarantine Ring 0 failures, fabric failures, CPU-card failures, or device failures.

See:

```text
docs/supervisor-card-v1.md
docs/management-anomaly-detection-v1.md
```

## Capability Model

Capabilities define specific resource authority.

Capabilities describe:

- Resource ownership.
- Allowed operations.
- Address windows.
- DMA permissions.
- Mailbox permissions.
- Device permissions.
- Revocation policy.

Capabilities prevent the ring model from becoming too coarse.

For example, two Ring 1 services may both be trusted services, but only the storage service should receive access to storage-controller DMA buffers.

See:

```text
docs/capability-model.md
```

## Ring and Capability Interaction

Access decisions should consider both ring and capability.

Example policy:

```text
Ring 0 + valid fabric capability
    -> may configure fabric service

Ring 1 + valid device capability
    -> may operate assigned device

Ring 2 + valid service capability
    -> may submit requests to assigned service

Ring 3 + no capability
    -> denied
```

Being in a privileged ring is not enough to access every resource.

Having a capability is not enough if the caller is executing in a ring that is not allowed to exercise that kind of capability.

## DMA Security

DMA access must be mediated.

DMA requests require:

1. Valid DMA capability.
2. Valid memory capability.
3. Valid ownership chain.
4. Ring policy allowing the request.

Future FPGA fabric components may participate in:

- DMA authorization.
- DMA isolation.
- DMA revocation.
- Fault reporting.
- Timeout enforcement.

No device should receive unrestricted DMA access to system memory.

See:

```text
docs/dma-engine-v1.md
```

## Device Trust Model

Devices receive explicit access grants.

No device should automatically gain unrestricted memory visibility or register access simply because it is physically installed.

Device discovery should describe:

- Device identity.
- Required capabilities.
- DMA needs.
- Mailbox endpoints.
- Interrupt sources.
- Driver binding requirements.

See:

```text
docs/discovery-rom-format.md
```

## Fabric Security

The FPGA fabric is an enforcement assistant.

The fabric may implement:

- Register access filtering.
- DMA bounds checking.
- Interrupt routing control.
- Mailbox endpoint control.
- Discovery visibility.
- Fault counters.
- Capability lookup acceleration.

The fabric should not become an undocumented black box.

Security-relevant fabric behavior must be described by stable register, mailbox, and discovery specifications.

## Multiprocessor Security

Future CPU modules should communicate through mediated interfaces.

Examples:

- Mailboxes.
- Shared queues.
- Controlled memory windows.
- Capability-backed DMA buffers.

Each CPU card should retain local bank-zero isolation.

A future secondary CPU should not automatically obtain Ring 0 authority merely because it is physically installed.

CPU-card identity, boot state, and assigned authority should be explicit.

## Accelerator Security

Accelerators are treated as powerful but untrusted participants.

They receive explicit capabilities and controlled access to resources.

Accelerator jobs should follow this pattern:

```text
1. Discover accelerator.
2. Allocate buffers.
3. Grant narrow memory capability.
4. Grant accelerator execution capability.
5. Submit job through mailbox or command queue.
6. Validate completion.
7. Revoke temporary capabilities.
```

Accelerators should never receive unrestricted access to CPU-local memory or kernel memory.

## Recovery and Fault Isolation

Security failures should degrade safely.

Possible recovery actions:

- Revoke capability.
- Disable DMA channel.
- Reset service.
- Reset slot.
- Quarantine card.
- Enter recovery mode.
- Preserve fault log for diagnosis.

The supervisor controller should assist recovery and fault isolation.

## Rev A Enforcement Strategy

Rev A does not need to implement full hardware isolation.

Rev A should establish the policy structure and make violations visible.

Recommended Rev A behavior:

- Define Rings 0-3 in software architecture.
- Keep supervisor outside the ring model.
- Require capabilities in firmware interfaces where practical.
- Prevent unrestricted DMA.
- Log denied or invalid accesses.
- Keep dangerous features disabled by default.
- Provide recovery mode.

## Future Enforcement Strategy

Later revisions may add:

- FPGA access-control tables.
- Hardware-assisted DMA bounds checking.
- MMU or window-mapping experiments.
- Per-ring memory windows.
- Capability-tagged mailbox endpoints.
- ABORT-based protection faults.
- Slot quarantine.

## Design Principle

ATARIX should not rely on a single security mechanism.

Use rings for broad execution privilege.

Use capabilities for precise resource authority.

Use the supervisor for recovery and accountability.

Use the FPGA fabric for mediation and enforcement assistance.

## Open Questions

- How Ring 0 through Ring 3 transitions are represented in the OS.
- Whether Ring 3 is enforced by software convention, memory windows, or future MMU logic.
- Whether ABORT should signal protection violations.
- How capabilities are stored and revoked.
- Whether capability tables live in system RAM, supervisor memory, or fabric memory.
- How secondary CPU cards acquire initial authority.
- Whether signed boot images are required before multi-user operation.