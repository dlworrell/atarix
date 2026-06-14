# ATX-SPEC-005 Mailbox Model

## Status

Draft stub v0.1

## Purpose

This document defines Atarix inter-object communication through mailboxes.

## Required Topics

- Mailboxes as objects
- Endpoint ownership
- Message structure
- Delivery semantics
- Ordering semantics
- Ring enforcement
- Capability enforcement
- Endpoint lifecycle
- Message validation
- Failure behavior
- Mailbox cleanup
- Remote mailbox behavior

## Required Questions

- Are mailboxes objects?
- Who owns a mailbox?
- Can a mailbox migrate?
- Is delivery reliable?
- Is delivery ordered?
- How are malformed messages handled?
- How are cross-ring messages mediated?

## Dependency

This specification depends on ATX-SPEC-001 Security Model, ATX-SPEC-002 Authority Model, and ATX-SPEC-003 Capability Model.
