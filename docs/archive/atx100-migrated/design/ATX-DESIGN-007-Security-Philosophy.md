---
document: ATX-DESIGN-007
title: Security Philosophy
type: Design Document
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
---

# ATX-DESIGN-007: Security Philosophy

## Purpose

This document defines the security philosophy behind Atarix.

## Principle

Atarix is deny-by-default and capability-oriented.

Names, handles, addresses, descriptors, cache entries, and lookup results may identify or accelerate access. They do not grant authority.

## Initial Security Rules

- Authority must be explicit.
- Capability checks must precede protected operations.
- Policy may further restrict capability-derived authority.
- Hardware may accelerate validation but may not bypass validation.
- Compatibility personalities must map foreign authority models into native Atarix authority rules.
