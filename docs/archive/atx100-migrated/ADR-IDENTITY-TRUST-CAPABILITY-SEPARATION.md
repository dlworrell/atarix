# ADR: Identity, Trust, and Capability Separation

Status: Accepted

## Context

Many systems implicitly treat identification as authorization.

Example:

```text
Identity
    => Authority
```

This creates tight coupling between discovery, provisioning, security, and resource access.

ATARIX is intended to support:

- factory hardware
- development hardware
- prototype hardware
- homebrew hardware
- future third-party hardware

without requiring changes to core platform architecture.

## Decision

ATARIX separates three concepts:

```text
Identity proves existence.
Trust determines authority.
Capabilities determine access.
```

### Identity

Identity answers:

```text
Who are you?
What are you?
Who built you?
```

Identity is provided by the Identity EEPROM.

Identity validation proves only that a board can identify itself and that the identity record is valid.

Identity alone grants no authority.

### Trust

Trust answers:

```text
How much confidence should the platform place in this device?
```

Trust is evaluated by supervisor policy.

Trust may be influenced by:

- provisioning state
- board class
- trust class
- development mode
- manufacturing records
- future cryptographic validation

Trust does not automatically grant access to resources.

### Capabilities

Capabilities answers:

```text
What may this device do?
```

Capabilities describe permissions and allowed operations.

Examples:

- DMA access
- boot authority
- recovery authority
- mailbox participation
- interrupt generation
- service publication

Capabilities are evaluated independently of identity.

## Consequences

The following states are legal:

```text
Identified but Untrusted
Trusted but Restricted
Homebrew but Allowed
Factory but Limited
```

This supports safe extensibility while preserving platform control.

## Architectural Benefits

- Supports homebrew cards without implicit trust.
- Supports future security enhancements.
- Reduces coupling between discovery and authorization.
- Allows policy evolution without changing hardware identity formats.
- Keeps provisioning, discovery, and capability systems independent.

## Related Specifications

- identity-eeprom-v1.md
- discovery-rom-format-v1.md
- capability-record-format-v1.md
- service-directory-format-v1.md
- factory-provisioning-v1.md

## Related Principles

```text
Identity Is A First-Class Service
Observability Is A Feature
Provisionability Is A Feature
Recovery Is Mandatory
```
