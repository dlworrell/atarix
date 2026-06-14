# Atarix Security Doctrine

## Status

Normative draft v0.1

## Purpose

This document defines the core security doctrine for Atarix. All specifications, subsystems, services, protocols, runtimes, and applications shall be reviewed against this doctrine.

The purpose of this doctrine is not merely to prevent compromise. The purpose is to ensure that compromise does not automatically become authority.

## Core Security Philosophy

Atarix assumes:

- Software defects exist.
- Remote code execution will occur.
- Services may be compromised.
- User identities may be stolen.
- Internal nodes are not inherently trusted.
- Networks are hostile.
- Cleanup failures become security failures.

Therefore, Atarix protects authority, not merely code.

## ARC-SEC Rules

1. Authority must never be inferred from execution.
2. Connectivity does not imply trust.
3. Compromise is expected and must be containable.
4. No ambient authority.
5. Explicit lifecycle.
6. No orphans.
7. Leases by default.
8. Crash recovery is architectural.
9. Persistence must be requested.
10. Authority cleanup.
11. Garbage must be observable.
12. The system is not exempt.
13. Minimize attack surface.
14. No privilege escalation by identity.
15. Identity theft is not authority theft.
16. Secure by default.
17. Fail closed.
18. Verifiable state.
19. Reproducible recovery.
20. Least persistence.

## Summary

- Authority is explicit.
- Trust is explicit.
- Ownership is explicit.
- Persistence is explicit.
- Cleanup is explicit.
- System objects are not exempt.
- Identity is not authority.
- Connectivity is not trust.
- Compromise is expected.
- Containment is mandatory.
