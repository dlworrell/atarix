# Atarix Architecture Review Checklist

## Purpose

This checklist is used before implementing or merging any Atarix subsystem change.

It exists to prevent implementation details from accidentally defining architecture.

## Core Checks

- ARC-001: Physical location exposure
- ARC-002: Internal implementation exposure
- ARC-003: Name and identity separation
- ARC-004: Ring boundary enforcement
- ARC-005: Lookup is not access
- ARC-006: Capability and ring interaction
- ARC-007: Single-node viability
- ARC-008: Many-node viability
- ARC-009: Object mobility
- ARC-010: Unnamed object support
- ARC-011: Name rebinding
- ARC-012: Human-readable interface
- ARC-013: Scriptability without hardware knowledge
- ARC-014: CPU assumption leakage
- ARC-015: Future compatibility
- ARC-016: Contract vs implementation
- ARC-017: CPU-card replaceability
- ARC-018: Fabric first
- ARC-019: No undocumented special cases
- ARC-020: Replaceability
- ARC-021: Cleanup and resource hygiene

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
