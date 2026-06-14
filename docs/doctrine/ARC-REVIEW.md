# Atarix Architecture Review Doctrine

## Purpose

This document defines the review process used to evaluate Atarix architecture and implementation changes.

Architecture review exists to prevent implementation details from becoming accidental doctrine.

Every new subsystem, service, protocol, runtime feature, kernel feature, and public API shall be reviewed against the architecture and security doctrine before implementation is accepted.

## Required Review Sections

Every architecture proposal and implementation pull request shall include:

```text
Affected Pillars:

Modified Pillars:

Relevant ARC Checks:

Security Doctrine Compliance:

Lifecycle and Cleanup Impact:

Authority Impact:

Persistence Impact:

Required Tests:

Known Exceptions:
```

## Pre-Code Review

Before implementation begins, answer:

1. Does this feature create authority?
2. Does this feature transfer authority?
3. Does this feature persist authority?
4. Does this feature expose authority?
5. Does this feature create new attack surface?
6. Does this feature create objects?
7. Does this feature create garbage?
8. Does this feature survive reboot?
9. Does this feature survive uninstall?
10. Does this feature comply with ARC-SEC?

If any answer is unknown, the specification must be written or updated before code is written.

## Human Comprehensibility

A system administrator should be able to answer:

- What is this object?
- Who owns it?
- Why does it exist?
- What authority does it possess?
- How do I remove it?

If those questions cannot be answered, the design is suspect.

Human-readable namespace and object observability are security features.

## Review Outcome

A review may result in:

- Accepted
- Accepted with required tests
- Returned for specification work
- Rejected due to architectural violation

## Foundational Modification Rule

If a proposal modifies a foundational pillar, architecture review is mandatory before implementation.

Foundational pillars currently include:

- Object Model
- Namespace Model
- Security Model
- Authority Model
- Capability Model
- Lifecycle Model

## Security Doctrine Rule

If a proposal violates any ARC-SEC rule, the violation must be explicitly documented and approved before merge.

Undocumented exceptions are not permitted.

## Implementation Detail Rule

Implementation details shall not be exposed as public contracts.

Examples of implementation details include:

- Table slots
- Memory addresses
- Mailbox numbers
- Internal indexes
- CPU-local pointers
- Physical node addresses
- Raw device locations

Public contracts should expose objects, names, OIDs, capabilities, messages, and documented APIs.
