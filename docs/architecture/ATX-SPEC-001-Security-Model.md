# ATX-SPEC-001 Security Model

## Status

Draft v0.1

## Purpose

This document defines the mandatory security architecture of the Atarix operating system and distributed object fabric.

The requirements in this document are normative.

All subsystems, services, protocols, runtimes, applications, and future specifications shall comply with this specification.

## Design Philosophy

Atarix assumes:

- Software defects exist.
- Services will be compromised.
- Remote code execution will occur.
- Identities may be stolen.
- Networks are hostile.
- Internal nodes are not inherently trusted.
- Cleanup failures become security failures.

Security is achieved through containment of authority rather than prevention of execution.

### Core Principle

Code execution does not imply authority.

A compromised process or object shall possess only the authority explicitly granted to it through capabilities, policy, ownership, and ring enforcement.

## Threat Model

Atarix specifically aims to reduce the impact of:

- Buffer overflows
- Use-after-free defects
- Memory corruption
- Parser compromise
- Zero-click exploitation
- Privilege escalation
- User identity theft
- Session theft
- Pivoting between services or nodes
- Stale authority
- Orphaned persistence
- Hidden cleanup failures

Atarix assumes that prevention mechanisms may fail.

The architecture must therefore limit what a compromised object can do after compromise.

## Trust Model

Trust is never inferred from:

- Network connectivity
- Physical locality
- User identity
- Process identity
- Object name
- Namespace visibility
- Execution context
- System status

Trust must be represented through explicit policy, capability, ownership, or ring mediation.

## Authority Model Overview

Authority is explicit.

Authority may be granted through:

- Object ownership
- Capabilities
- Explicit delegation
- System policy
- Ring-mediated control paths

Authority shall not be inferred from execution, identity, location, or connectivity.

## Ring Architecture

Rings provide coarse-grained security boundaries.

Capabilities provide fine-grained authority within those boundaries.

A capability shall not bypass ring restrictions unless explicitly permitted by the security model.

Lower rings shall not directly access higher rings.

Cross-ring communication must be mediated.

## Capability Enforcement

Capabilities represent explicit authority.

A capability shall be:

- Scoped
- Revocable
- Auditable
- Bound to intended authority
- Constrained by ring policy

Possessing an object name or identity shall not imply possession of a capability.

## Containment Requirements

Compromise of one object shall not imply compromise of unrelated objects.

Compromise of a service shall not imply authority over:

- The directory service
- The namespace service
- The object manager
- Storage
- Devices
- Network fabric
- Other users
- Other services

Containment is mandatory.

## Attack Surface Requirements

Objects and services shall expose the smallest practical interface.

Default state shall be closed, private, unpublished, and inaccessible unless explicitly opened.

Debug endpoints, administrative interfaces, broad control channels, and unused services shall not be enabled by default.

## Identity Theft Requirements

A stolen user identity, session label, process identity, node identity, or object name shall not automatically grant broad authority.

Identity labels may identify subjects.

Capabilities and policy grant authority.

Sensitive authority should be compartmentalized, scoped, expiring, revocable, and revalidated where appropriate.

## Lifecycle Security

Every object and resource shall have:

- Owner
- Scope
- Lifetime
- Cleanup authority
- Persistence policy
- Audit visibility

Objects shall not become orphaned silently.

Temporary resources should be lease-based by default.

## Persistence Rules

Persistence shall be explicit.

The default state is ephemeral.

Persistent objects, credentials, tokens, namespace bindings, caches, logs, and resources must have defined owner, retention reason, removal path, and audit visibility.

## Cleanup Requirements

Authority cleanup is mandatory.

When an object, user, application, service, node, or capability is removed or revoked, Atarix must identify and reconcile related:

- Capabilities
- Delegations
- Mailboxes
- Endpoints
- Sessions
- Directory entries
- Namespace bindings
- Persistent resources
- Temporary resources

Cleanup may destroy, transfer, revoke, or explicitly preserve state by policy.

## System Object Requirements

System objects are not exempt.

System objects may possess greater authority, but they remain subject to:

- Ownership
- Lifecycle
- Audit
- Scope
- Persistence
- Cleanup
- Review

Nothing exists merely because it is system.

## Failure Semantics

Atarix shall fail closed.

If policy, authority, capability, identity, directory, namespace, lifecycle, or security state cannot be verified, the operation shall be denied.

Failure shall not create authority.

## Auditability Requirements

The system should be able to answer:

- What authority exists?
- Who owns it?
- Who granted it?
- When was it granted?
- Is it still valid?
- What keeps it alive?
- What will remove it?

Security state should be inspectable and auditable.

## Recovery Requirements

Recovery from crash, failure, reboot, partition, or compromise should be deterministic.

Recovery shall reconcile:

- Object ownership
- Object lifecycle state
- Capability validity
- Authority delegation
- Namespace bindings
- Directory state
- Leases
- Persistence

## Security Doctrine Compliance

This specification incorporates the security doctrine defined in `docs/doctrine/ARC-SEC.md`.

All ARC-SEC rules are normative for this specification.

## Initial ARC-SEC Requirements

- ARC-SEC-001: Authority must never be inferred from execution.
- ARC-SEC-002: Connectivity does not imply trust.
- ARC-SEC-003: Compromise is expected and must be containable.
- ARC-SEC-004: No ambient authority.
- ARC-SEC-005: Explicit lifecycle.
- ARC-SEC-006: No orphans.
- ARC-SEC-007: Leases by default.
- ARC-SEC-008: Crash recovery is architectural.
- ARC-SEC-009: Persistence must be requested.
- ARC-SEC-010: Authority cleanup.
- ARC-SEC-011: Garbage must be observable.
- ARC-SEC-012: The system is not exempt.
- ARC-SEC-013: Minimize attack surface.
- ARC-SEC-014: No privilege escalation by identity.
- ARC-SEC-015: Identity theft is not authority theft.
- ARC-SEC-016: Secure by default.
- ARC-SEC-017: Fail closed.
- ARC-SEC-018: Verifiable state.
- ARC-SEC-019: Reproducible recovery.
- ARC-SEC-020: Least persistence.

## Open Questions

Q-001:
Which ring names and numeric values are normative?

Q-002:
Which system objects possess bootstrap authority?

Q-003:
How are authority records serialized for recovery?

Q-004:
What minimum audit record format is required for security state?
