# ATARIX-DEV-001: Development Principles

Status: Draft
Owner: ATARIX
Scope: hardware, FPGA, firmware, operating-system, tooling, and documentation changes

## Purpose

This document defines how ATARIX is developed.

ATARIX is not only a collection of source files. It is a machine architecture, a set of protocols, a hardware/software contract, and a long-lived engineering record. Development must therefore prefer systems that are understandable, observable, auditable, recoverable, and secure over systems that are merely clever.

Architecture decisions are influenced by four major traditions:

- Vega816 and BB816 practical 65C816 engineering;
- Sun, NuBus, and UPA workstation architecture;
- curl development discipline;
- OpenBSD security philosophy.

## Development Order

ATARIX development follows this order:

1. Architecture first.
2. Specification second.
3. Implementation third.

No major hardware, FPGA, firmware, operating-system, protocol, tool, or externally visible interface should exist without a corresponding specification.

An implementation may be exploratory, but exploratory work must be clearly marked as such and must not be treated as an architectural commitment until the relevant specification is written.

## Documentation First

Documentation is part of the engineering system, not a post-processing step.

A change that introduces or materially changes an interface must update the corresponding specification in the same change series. A separate future documentation update is not acceptable for committed interface behavior.

Required documentation may include:

- architecture overview;
- component specification;
- protocol specification;
- register map;
- memory map;
- packet or descriptor format;
- failure-mode description;
- recovery behavior;
- ADR.

## Version Everything

Interfaces are versioned.

Examples:

- Discovery Format v1;
- Mailbox Protocol v1;
- DMA Descriptor v1;
- Fabric Controller v1.

Existing interfaces must not be silently changed. A change to an existing interface must do one of the following:

- preserve compatibility;
- add a new version;
- document the compatibility break and migration path;
- explicitly mark the old interface as deprecated.

## Small Changes

Prefer:

- small commits;
- small pull requests;
- small reviews;
- small testable units.

Large architectural changes must be decomposed into reviewable units.

A commit should normally represent one logical change. Mechanical refactors, generated updates, and behavior changes should not be mixed without an explicit rationale.

## curl-Style Check-In Discipline

ATARIX adopts curl-style check-in discipline where practical.

A change should be easy to review, easy to test, easy to revert, and useful to future `git bisect` work.

Commit subjects should normally use an area prefix:

```text
area: imperative summary
```

Examples:

```text
discovery: validate image length before CRC copy
mailbox: add v1 CRC mismatch fixture
aems: show review findings in aggregate report
atarix-dev: add development principles
```

A change should include, or explicitly justify the absence of:

- tests;
- updated documentation;
- interface version impact;
- recovery or failure-mode impact;
- security or trust-boundary impact.

Reviewers should reject clever code that is hard to reason about when a clearer design is available.

## Observable Systems

Every subsystem should expose enough state for diagnosis and recovery.

Subsystem specifications should define, as applicable:

- status;
- counters;
- fault history;
- health metrics;
- recovery information;
- diagnostic visibility;
- debug hooks or trace points.

Debuggability is a first-class design goal.

A subsystem that can fail silently is incomplete.

## Security by Design

ATARIX borrows from OpenBSD's security posture:

- secure by default;
- least privilege;
- privilege separation;
- capability revocation;
- explicit trust boundaries;
- small trusted computing bases;
- cryptography by reuse, not invention.

Security-sensitive specifications must identify trust boundaries and the authorities that cross them.

## Cryptographic Standards

Preferred primitives are:

- Ed25519 for signatures;
- ChaCha20-Poly1305 for authenticated encryption;
- BLAKE2b and SHA-256 for hashing.

Custom cryptography is not acceptable without an ADR and a specific review plan.

Cryptographic code should be reused from established implementations wherever possible.

## Recovery First

Every subsystem should define:

- failure modes;
- recovery behavior;
- reset behavior;
- diagnostic visibility;
- expected operator or supervisor action.

The machine should fail visibly and recover deliberately.

## Architecture Decision Records

Major decisions must be documented as ADRs.

Examples:

- why ULX3S was selected;
- why RP2350 was selected;
- why the CPU bus remains local;
- why capabilities are used;
- why the supervisor is outside the ring model.

An ADR should record:

- context;
- decision;
- alternatives considered;
- consequences;
- status;
- date.

## Ring and Capability Model

ATARIX uses:

- rings for execution authority;
- capabilities for resource authority.

Neither mechanism is sufficient alone.

Specifications that grant access to resources must identify whether the authority comes from execution ring, capability possession, supervisor policy, or some combination.

## Acceptance Expectations

A significant ATARIX change should be rejected or sent back for design work when it lacks:

- a specification for a new externally visible behavior;
- a versioning answer for an interface change;
- tests or a test rationale;
- an observability answer for a new subsystem;
- a recovery answer for a fault-prone subsystem;
- a trust-boundary answer for a security-sensitive path;
- an ADR for a major architecture decision.

## Design Principle

Prefer systems that are understandable, observable, auditable, recoverable, and secure over systems that are merely clever.
