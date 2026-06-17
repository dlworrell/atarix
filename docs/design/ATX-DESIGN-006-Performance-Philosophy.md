---
document: ATX-DESIGN-006
title: Performance Philosophy
type: Design Document
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
---

# ATX-DESIGN-006: Performance Philosophy

## Purpose

This document defines how Atarix evaluates and applies performance optimizations.

## Principle

Performance is valuable only when it preserves architecture semantics.

Optimizations may reduce latency, memory use, copy cost, or CPU overhead. They must not weaken authority, policy, audit, recovery, human readability, or compatibility behavior.

## Initial Rules

- Acceleration is not authority.
- Cache state is not authority.
- Reachability is not permission.
- Fast paths must preserve slow-path semantics.
- Benchmarks should include correctness and auditability evidence.
