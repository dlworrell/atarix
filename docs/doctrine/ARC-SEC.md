# Atarix Security Doctrine

## Status

Normative draft v0.1

## Purpose

This document defines the core security doctrine for Atarix.

All Atarix specifications, subsystems, services, protocols, runtimes, and applications shall be reviewed against this doctrine.

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

Code execution shall not imply authority.

Connectivity shall not imply trust.

Identity shall not imply authority.

Persistence shall not be accidental.

## ARC-SEC-001: Authority Must Never Be Inferred From Execution

Running code does not imply authority.

An object may execute only with the explicit authority granted to it by capability, ownership, policy, or ring mediation.

## ARC-SEC-002: Connectivity Does Not Imply Trust

The ability to communicate with another object, service, node, or device does not imply authority to control it.

Reachability is not permission.

## ARC-SEC-003: Compromise Is Expected And Must Be Containable

Atarix assumes that services, applications, and devices may be compromised.

A compromise shall be contained to the compromised object's explicit authority.

## ARC-SEC-004: No Ambient Authority

Authority shall not be granted implicitly through process identity, user identity, filesystem location, network location, or execution context.

Authority must be explicit, scoped, revocable, and auditable.

## ARC-SEC-005: Explicit Lifecycle

Every object and resource shall have an owner, scope, lifetime, cleanup authority, and persistence policy.

## ARC-SEC-006: No Orphans

No object or resource may exist without ownership.

If an owner is destroyed, removed, or revoked, the owned resources must be destroyed, transferred, or explicitly preserved by policy.

## ARC-SEC-007: Leases By Default

Temporary resources should use leases by default.

Examples include sessions, temporary mailboxes, discovery records, locks, caches, reservations, and temporary storage.

Expired leases shall trigger cleanup.

## ARC-SEC-008: Crash Recovery Is Architectural

Cleanup after crash, reboot, failure, or partition is part of the architecture.

Recovery shall reconcile object ownership, leases, authority, namespace bindings, and persistence state.

## ARC-SEC-009: Persistence Must Be Requested

Persistence is not the default.

An object, credential, cache, token, service, namespace binding, or resource shall persist only when persistence is explicitly requested and justified by policy.

## ARC-SEC-010: Authority Cleanup

When an object, application, user, service, node, or capability is removed or revoked, related authority must be cleaned up.

This includes capabilities, delegations, mailboxes, endpoints, directory bindings, namespace entries, sessions, and persistent state.

## ARC-SEC-011: Garbage Must Be Observable

Atarix must be able to answer:

- Who owns this?
- Why does it exist?
- When was it created?
- When was it last used?
- What authority keeps it alive?
- What will remove it?

Unobservable garbage is architecturally suspect.

## ARC-SEC-012: The System Is Not Exempt

System objects may possess greater authority, but they are not exempt from ownership, lifecycle, audit, cleanup, scope, or persistence rules.

Nothing exists merely because it is system.

## ARC-SEC-013: Minimize Attack Surface

Every object shall expose the smallest necessary interface.

Unused services, unnecessary endpoints, broad APIs, debug interfaces, and implicit management channels shall be absent by default.

## ARC-SEC-014: No Privilege Escalation By Identity

A user ID, process ID, login session, object name, or node identity is not authority.

Identity may label a subject, but authority still requires explicit permission.

## ARC-SEC-015: Identity Theft Is Not Authority Theft

Stealing identity, session labels, or user identifiers shall not automatically grant all authority associated with the user.

Authority shall be compartmentalized, scoped, expiring, revocable, and revalidated for sensitive actions.

## ARC-SEC-016: Secure By Default

The default state shall be the safest state.

New objects are private.

New services are unpublished.

New endpoints are inaccessible.

New nodes are untrusted.

New capabilities have minimal rights.

## ARC-SEC-017: Fail Closed

If policy, authority, capability, identity, directory, or security state cannot be verified, the operation shall be denied.

Failure shall not create authority.

## ARC-SEC-018: Verifiable State

The system shall be able to inspect and verify current authority, ownership, persistence, lifecycle, and trust state.

Security state should be queryable and auditable.

## ARC-SEC-019: Reproducible Recovery

Recovery from crash, failure, or compromise should be deterministic.

Given policies, object state, persistent records, and authority records, Atarix should be able to reconstruct intended state without undocumented administrator knowledge.

## ARC-SEC-020: Least Persistence

Objects, credentials, sessions, caches, logs, tokens, namespace bindings, and authority records should live no longer than necessary.

Before asking whether something can persist, the system shall ask whether it should persist.

## Summary

The Atarix security doctrine can be summarized as:

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
