---
document: ATX-100-CH15
title: Lookup Acceleration
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-020
implements:
  - include/atarix/index.h
  - rtl/atarix/atx_spec_020_accelerator.v
verified_by:
  - atx_spec_020_accelerator_ci_tb
  - atx_spec_020_modules_tb
---

# Chapter 15: Lookup Acceleration

## Principle

Lookup acceleration is not authority.

Atarix uses accelerated lookup structures to reduce latency, memory movement, cache misses, synchronization cost, and search cost. These structures may accelerate discovery, filtering, routing, indexing, or selection.

They do not grant permission.

## Canonical Rule

A lookup result identifies a candidate target. It does not authorize access to that target.

Authority still derives from:

- Capability state.
- Policy state.
- Native object state.
- Lifecycle state.
- Recovery state.
- Audit rules.

## Human-Readable Projection

Every accelerated structure must provide an explainable projection suitable for audit and debugging.

Examples:

```text
Runtime hash table -> registry view
Compressed index -> ordered generation window
Bitmap query -> explicit result set
Membership filter -> precheck explanation
```

## Audit Window Requirement

Every accelerated lookup must be reconstructable inside a bounded audit window.

An audit window should answer:

- Who requested the lookup?
- What key was used?
- Which structure was used?
- What version and generation were involved?
- What candidate was returned?
- What authority check followed?
- What policy decision followed?

## Hardware Acceleration

ATX-SPEC-020 defines an initial RTL-backed lookup acceleration path.

The implementation includes SIMD-style control byte probing, scalar comparison reference behavior, Krapivin-style stepping, Elias-Fano-style decoding, and hardware audit-window support.

The RTL simulations validate both correctness and a scalar-vs-SIMD gain proxy.

## Relationship To ATX-SPEC-020

This chapter owns the stable architectural explanation of lookup acceleration.

ATX-SPEC-020 remains responsible for detailed payload definitions, RTL module interfaces, simulation behavior, and verification evidence.
