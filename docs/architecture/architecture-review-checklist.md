# Atarix Architecture Review Checklist

## Purpose

This checklist is used before implementing or merging any Atarix subsystem change.

It exists to prevent implementation details from accidentally defining architecture.

Every deliverable shall state which checks are affected and whether the design passes them.

## ARC-001: Physical Location Exposure

Does this expose physical location?

Examples of violations:

```text
Node 7
Mailbox 42
Memory 0x1234
CPU 0 local pointer
```

Public architecture should expose objects, names, OIDs, capabilities, and contracts instead.

## ARC-002: Internal Implementation Exposure

Does this expose table indexes, slots, raw handles, or private layout?

Implementation details must remain behind contracts.

## ARC-003: Name and Identity Separation

Does this couple names and identities?

Names are bindings.

Object identities are immutable opaque identifiers.

## ARC-004: Ring Boundary Enforcement

Can a lower ring directly access a higher ring?

The answer must be no unless access is explicitly mediated by the security doctrine.

## ARC-005: Lookup Is Not Access

Does lookup imply access?

The answer must be no.

Lookup discovers an object identity.

Authority is still checked separately.

## ARC-006: Capability and Ring Interaction

Can capability delegation bypass ring rules?

The answer must be no.

Capabilities provide fine-grained authority within coarse ring boundaries.

## ARC-007: Single-Node Viability

Would this work on a single-node Atarix system?

## ARC-008: Many-Node Viability

Would this work across many nodes?

If the design changes substantially between one node and many nodes, locality assumptions are leaking.

## ARC-009: Object Mobility

Would this still work if the object moved?

If not, identity or location has leaked into the contract.

## ARC-010: Unnamed Object Support

Can the object exist unnamed?

If not, name and identity may be improperly coupled.

## ARC-011: Name Rebinding

Can a name move to another object?

If not, the binding model may be violated.

## ARC-012: Human-Readable Interface

Would a user understand the object being referenced?

Prefer meaningful names and object abstractions over numeric implementation details.

## ARC-013: Scriptability Without Hardware Knowledge

Can a user script this without knowing hardware topology?

If not, hardware details are leaking.

## ARC-014: CPU Assumption Leakage

Does this leak CPU assumptions?

The design must not assume:

- A specific CPU word size
- A specific endian model
- An MMU
- Cache coherence
- Atomic instructions
- Flat memory
- Direct device access

A subsystem may require platform support, but platform assumptions must not leak into the public architecture.

## ARC-015: Future Compatibility

Would this design still work in a larger Atarix system with multiple CPUs, boards, racks, or sites?

## ARC-016: Contract vs Implementation

Am I exposing an implementation detail instead of a contract?

Contracts are stable.

Implementations are replaceable.

## ARC-017: CPU Card Replaceability

Can this subsystem survive a CPU-card replacement?

The architecture should remain unchanged when the CPU card changes.

Only the CPU-card support package should change.

## ARC-018: Fabric First

Would this still work if the object lived on another node?

Avoid direct pointer passing, shared address space assumptions, and raw memory references in public contracts.

Prefer OIDs, capabilities, mailboxes, and messages.

## ARC-019: No Undocumented Special Cases

Does this create a special case for kernel, supervisor, or fabric objects?

Special authority must be explicit, documented, and auditable.

## ARC-020: Replaceability

Can this subsystem be replaced without changing its clients?

Clients should depend on contracts, not implementation details.

## Required Review Questions

Every subsystem proposal must answer:

```text
Affected Pillars:

Modified Pillars:

Relevant ARC Checks:

Known Exceptions:

Required Tests:
```

If a foundational pillar is modified, architecture review is required before implementation.
