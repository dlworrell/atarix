# ADR-0014: Testing and Verification Strategy

Status: Accepted

Date: 2026-06-10

## Context

ATARIX combines hardware, firmware, FPGA logic, protocols, security mechanisms, and operating-system research.

Verification must be treated as a required engineering activity.

## Decision

Testing and verification are first-class architectural requirements.

All major architectural interfaces should have documented tests, validation procedures, or simulation coverage.

The project testing authority is:

```text
docs/testing-strategy.md
```

## Principles

- Test behavior rather than implementation details.
- Preserve reproducibility.
- Version test formats.
- Prefer small targeted tests.
- Support randomized testing where practical.
- Record sufficient state to diagnose failures.
- Keep architectural tests portable across implementations.

## Scope

Testing applies to:

- CPU behavior.
- Firmware.
- FPGA fabric.
- Discovery records.
- Mailbox protocols.
- DMA descriptors.
- Capability records.
- Supervisor services.
- Recovery mechanisms.
- Hardware bring-up.
- Integration workflows.

## Consequences

Pros:

- Earlier defect discovery.
- Better documentation.
- Easier hardware bring-up.
- Safer architectural evolution.

Cons:

- Additional engineering effort.
- Additional tooling.
- Additional CI infrastructure.

## Design Rule

If a subsystem cannot be verified, it is not complete.

Observability and testability are architectural features.