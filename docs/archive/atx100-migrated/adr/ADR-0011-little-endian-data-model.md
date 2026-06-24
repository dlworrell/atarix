# ADR-0011: Little-Endian Data Model

Status: Accepted

Date: 2026-06-10

## Context

ATARIX begins with a W65C816 CPU but is expected to grow into a larger fabric-based workstation architecture with memory services, accelerators, large identifiers, and long-lived persistent resources.

The project requires a consistent binary representation.

## Decision

All architecture-defined binary structures use little-endian encoding.

The architecture may use 8-bit, 16-bit, 24-bit, 32-bit, and 64-bit fields as appropriate.

CPU width is not system width.

## Evidence

- Natural fit with W65C816 memory representation.
- Simplifies firmware implementation.
- Simplifies protocol definition.
- Avoids mixed-endian ambiguity.
- Supports future 64-bit identifiers and timestamps.

## Alternatives Considered

- Big-endian architecture.
- Mixed-endian structures.
- 16-bit-only architecture.
- 32-bit-only architecture.
- 64-bit-only architecture.

## Consequences

Pros:
- Consistent binary representation.
- Future-proof identifiers and timestamps.
- Easier protocol evolution.

Cons:
- Requires explicit rules for large values on an 8/16-bit CPU.
- Requires documentation of multi-byte register access semantics.

## Future Revisions

Future formats may define 128-bit identifiers if required, but the little-endian rule should remain stable.