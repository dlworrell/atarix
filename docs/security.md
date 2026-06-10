# ATARIX Security Architecture

## Principle

Devices are not trusted by default.

Security is designed into the architecture rather than added later.

## Capability Model

Capabilities define:

- Resource ownership
- Allowed operations
- Address windows
- DMA permissions
- Revocation policy

## DMA Security

DMA access must be mediated.

Future FPGA fabric components may participate in:

- DMA authorization
- DMA isolation
- DMA revocation

## Device Trust Model

Devices receive explicit access grants.

No device should automatically gain unrestricted memory visibility.

## Multiprocessor Security

CPU modules should communicate through mediated interfaces.

Examples:

- Mailboxes
- Shared queues
- Controlled memory windows

## Accelerator Security

Accelerators are treated as powerful but untrusted participants.

They receive explicit capabilities and controlled access to resources.

## Recovery

Security failures should degrade safely.

The supervisor controller should assist recovery and fault isolation.