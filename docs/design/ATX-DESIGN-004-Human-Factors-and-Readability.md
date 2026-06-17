---
document: ATX-DESIGN-004
title: Human Factors and Readability
type: Design Document
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
---

# ATX-DESIGN-004: Human Factors and Readability

## Purpose

This document defines the human-readability requirements that guide Atarix architecture and implementation.

## Principle

Systems that cannot be understood cannot be reliably audited, repaired, or evolved.

Atarix values explicitness, determinism, traceability, and readable projections even when machine-optimized paths are used.

## Initial Requirements

- Optimized structures must provide explainable projections.
- Audit windows must be readable by humans.
- Namespaces, capabilities, services, and objects should expose reviewable state.
- Generated artifacts must identify their sources.
- Compatibility layers must explain native authority mappings.
