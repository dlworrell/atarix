# Atarix Architecture Review Doctrine

## Purpose

This document defines the review process used to evaluate Atarix architecture and implementation changes.

Architecture review exists to prevent implementation details from becoming accidental doctrine.

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

Human-readable namespace and object observability are security features.

## Implementation Detail Rule

Implementation details shall not be exposed as public contracts.

Examples include table slots, memory addresses, mailbox numbers, internal indexes, CPU-local pointers, physical node addresses, and raw device locations.
