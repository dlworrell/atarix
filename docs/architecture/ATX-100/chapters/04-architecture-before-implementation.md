# Chapter 4: Architecture Before Implementation

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/architecture-review-checklist.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/architecture-review-checklist.md -->
### Integrated source: `docs/architecture/architecture-review-checklist.md`

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

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/architecture-review-template.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/architecture-review-template.md -->
### Integrated source: `docs/reviews/architecture-review-template.md`

# Atarix Architecture Review Template

## Review Target

Name:

Type:

Status:

Related Specification:

Related Pull Request:

## Affected Pillars

```text
Object Model:
Namespace Model:
Security Model:
Authority Model:
Capability Model:
Lifecycle Model:
Mailbox Model:
Resource Model:
Error Model:
Audit Model:
Time Model:
Versioning Model:
Portability Model:
Test Model:
```

## Security Doctrine Compliance

Mark each item PASS, FAIL, N/A, or NEEDS REVIEW.

```text
ARC-SEC-001 Authority not inferred from execution:
ARC-SEC-002 Connectivity does not imply trust:
ARC-SEC-003 Compromise remains contained:
ARC-SEC-004 No ambient authority:
ARC-SEC-005 Explicit lifecycle:
ARC-SEC-006 No orphaned objects:
ARC-SEC-007 Lease behavior defined:
ARC-SEC-008 Crash recovery defined:
ARC-SEC-009 Persistence justified:
ARC-SEC-010 Authority cleanup defined:
ARC-SEC-011 Garbage observable:
ARC-SEC-012 System objects comply:
ARC-SEC-013 Attack surface minimized:
ARC-SEC-014 No identity-based escalation:
ARC-SEC-015 Identity theft is not authority theft:
ARC-SEC-016 Secure defaults:
ARC-SEC-017 Fail closed:
ARC-SEC-018 State auditable:
ARC-SEC-019 Recovery reproducible:
ARC-SEC-020 Least persistence:
```

## Authority Review

How is authority granted?

How is authority revoked?

Can authority be inferred?

Can authority be escalated?

## Lifecycle Review

Who owns each created object?

What is each object's lifetime?

What cleans it up?

What happens after crash, uninstall, removal, or revocation?

## Required Tests

```text
Unit tests:
Integration tests:
Security tests:
Regression tests:
Failure tests:
Lifecycle cleanup tests:
```

## Review Decision

```text
Decision: ACCEPTED / ACCEPTED WITH TESTS / RETURNED / REJECTED
Reviewer:
Date:
Notes:
```
