# Atarix Architecture Review Template

## Review Target

Name:

Type:

Status:

Related Specification:

Related Pull Request:

## Affected Pillars

List all pillars this change depends upon.

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

## Modified Pillars

List all pillars this change modifies.

```text
Modified Pillars:
```

If any foundational pillar is modified, architecture review is mandatory before implementation.

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

Can identity theft become authority theft?

## Trust Review

What trust assumptions exist?

Can compromise spread?

Can connectivity imply control?

Can location imply trust?

## Lifecycle Review

Who owns each created object?

What is each object's lifetime?

What cleans it up?

What happens after crash?

What happens after uninstall or removal?

What happens after revocation?

## Persistence Review

What state persists?

Why does it persist?

Who owns persisted state?

How is persisted state removed?

How is persisted state audited?

## Attack Surface Review

What new interfaces are exposed?

What new messages are accepted?

What new endpoints are created?

What input validation is required?

What is disabled by default?

## Failure Review

What happens when policy lookup fails?

What happens when capability validation fails?

What happens when directory lookup fails?

What happens during crash, reboot, or partition?

Does failure create authority?

## Required Tests

List required tests before merge.

```text
Unit tests:
Integration tests:
Security tests:
Regression tests:
Failure tests:
Lifecycle cleanup tests:
```

## Known Exceptions

List and justify any exception to the doctrine.

```text
Exception:
Justification:
Risk:
Mitigation:
Review owner:
```

## Review Decision

```text
Decision: ACCEPTED / ACCEPTED WITH TESTS / RETURNED / REJECTED
Reviewer:
Date:
Notes:
```
