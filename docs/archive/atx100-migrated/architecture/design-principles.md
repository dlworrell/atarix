# Atarix Design Principles

## Purpose

This document defines the guiding principles that govern Atarix architectural and implementation decisions.

## Principles

1. Objects first.
2. Human names first.
3. Authority is explicit.
4. Security is layered.
5. Distribution is native.
6. Hardware independence is required.
7. Testability comes first.
8. Architecture precedes implementation.

## Objects First

Everything visible within Atarix is represented as an object: services, devices, mailboxes, storage entities, fabric nodes, and kernel facilities.

## Human Names First

Public interfaces expose human-readable names. Numeric handles, table indexes, memory addresses, and slot numbers are implementation details.

## Authority Is Explicit

Every operation must have a traceable source of authority. Authority must never arise from undocumented special cases.

## Hardware Independence

Core architecture shall not depend on a specific processor family, memory-management model, address size, or bus architecture.
