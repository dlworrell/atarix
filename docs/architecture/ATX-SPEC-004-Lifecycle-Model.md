# ATX-SPEC-004 Lifecycle Model

## Status

Draft stub v0.1

## Purpose

This document will define object creation, initialization, activation, suspension, destruction, cleanup, persistence, lease behavior, and crash recovery.

## Required Topics

- Object lifecycle states
- State transitions
- Ownership lifecycle
- Lease semantics
- Expiration
- Cleanup authority
- Persistence policy
- Uninstall behavior
- Crash recovery
- Garbage observability
- No-orphan enforcement

## Required Questions

- What states may every object occupy?
- Who may transition an object between states?
- What happens when an owner disappears?
- What happens when a lease expires?
- What survives reboot?
- What is removed after application uninstall?

## Dependency

This specification depends on ATX-SPEC-001 Security Model and ATX-SPEC-002 Authority Model.
