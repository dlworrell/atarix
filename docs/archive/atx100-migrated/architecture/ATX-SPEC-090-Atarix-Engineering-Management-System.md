---
document: ATX-SPEC-090
title: Atarix Engineering Management System
type: Specification
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
---

# ATX-SPEC-090: Atarix Engineering Management System

## Purpose

This specification defines AEMS, the Atarix Engineering Management System.

AEMS manages repository-native engineering data, including specifications, design documents, architecture reviews, issues, implementation references, tests, engineering gates, releases, and generated documentation.

## Initial Scope

AEMS SHALL support:

- Document discovery
- Metadata parsing
- Document lifecycle validation
- Specification registry generation
- Dependency graph validation
- Traceability graph validation
- Engineering Gate integration
- Documentation book building

## Non-Goals For v0.1

AEMS v0.1 does not own GitHub Projects synchronization, release publication, or generated PDF production.

## Relationship To ATX-100

AEMS supports ATX-100 by generating document maps, dependency graphs, traceability matrices, and release evidence appendices.
