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
