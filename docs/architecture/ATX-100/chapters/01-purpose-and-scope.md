---
document: ATX-100-CH01
title: Purpose and Scope
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-DESIGN-001
  - ATX-DESIGN-002
---

# Chapter 1: Purpose and Scope

## Purpose

Atarix is an architecture-first computing system that combines retro-compatible processor experimentation with modern object, capability, mailbox, fabric, audit, and engineering-management concepts.

The purpose of ATX-100 is to provide the canonical architectural overview of Atarix.

This chapter defines the scope of the architecture manual and explains how it relates to working specifications, reviews, implementation files, tests, and engineering gates.

## System Scope

Atarix includes:

- A native object model.
- Explicit capability-based authority.
- Mailbox-mediated communication.
- Directory and discovery services.
- Fabric-mediated hardware services.
- Compatibility personalities.
- Audit, recovery, lifecycle, and versioning models.
- RTL and software implementation evidence.
- AEMS-managed documentation and traceability.

## Non-Goals

ATX-100 does not replace detailed specifications.

Detailed register layouts, binary packet formats, RTL timing, C API specifics, and test vectors remain in the ATX-SPEC series or implementation-specific documentation.

## Canonical Architecture

ATX-100 owns stable architectural concepts.

A working specification may introduce a concept. After review and verification, stable explanatory material may be integrated into ATX-100. The originating specification remains as implementation detail or is archived for historical traceability.
