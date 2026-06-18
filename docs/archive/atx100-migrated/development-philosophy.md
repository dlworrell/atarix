# ATARIX Development Philosophy

## Purpose

This document defines how ATARIX is developed.

Architecture decisions are influenced by four major traditions:

- Vega816 and BB816 practical 65C816 engineering
- Sun/NuBus/UPA workstation architecture
- curl development discipline
- OpenBSD security philosophy

## Documentation First

Architecture first.

Specification second.

Implementation third.

No major hardware, FPGA, firmware, or operating-system component should exist without a corresponding specification.

## Version Everything

Interfaces are versioned.

Examples:

- Discovery Format v1
- Mailbox Protocol v1
- DMA Descriptor v1
- Fabric Controller v1

Existing interfaces should not be silently changed.

## Small Changes

Prefer:

- Small commits
- Small pull requests
- Small reviews

Large architectural changes should be decomposed into reviewable units.

## Observable Systems

Every subsystem should expose:

- Status
- Counters
- Fault history
- Health metrics
- Recovery information

Debuggability is a first-class design goal.

## Security by Design

Borrowing from OpenBSD:

- Secure by default
- Least privilege
- Privilege separation
- Capability revocation
- Explicit trust boundaries
- Cryptography by reuse, not invention

## Cryptographic Standards

Preferred primitives:

- Ed25519 for signatures
- ChaCha20-Poly1305 for authenticated encryption
- BLAKE2b and SHA-256 for hashing

Avoid custom cryptography.

## Recovery First

Every subsystem should define:

- Failure modes
- Recovery behavior
- Reset behavior
- Diagnostic visibility

The machine should fail visibly and recover deliberately.

## Architecture Decision Records

Major decisions should be documented as ADRs.

Examples:

- Why ULX3S was selected
- Why RP2350 was selected
- Why the CPU bus remains local
- Why capabilities are used
- Why the supervisor is outside the ring model

## Ring and Capability Model

ATARIX uses:

- Rings for execution authority
- Capabilities for resource authority

Neither mechanism is sufficient alone.

## Design Principle

Prefer systems that are understandable, observable, auditable, recoverable, and secure over systems that are merely clever.