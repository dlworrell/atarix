---
document: ATX-100-CH09
title: System Layers
type: Architecture Chapter
status: Draft
lifecycle: Draft
version: 0.1
canonical_owner: ATX-100
related:
  - ATX-SPEC-001
  - ATX-SPEC-002
  - ATX-SPEC-003
  - ATX-SPEC-005
  - ATX-SPEC-015
---

# Chapter 9: System Layers

## Layer Model

Atarix is organized as layered architecture rather than a monolithic bus-oriented machine.

```text
Applications
  -> Compatibility Personalities
  -> Native Services
  -> Object Model
  -> Capability and Policy Enforcement
  -> Mailbox Transport
  -> Fabric Services
  -> Hardware Modules
```

## Architectural Intent

The layer model prevents implementation convenience from becoming architectural authority.

A higher layer may request service from a lower layer. It may not bypass native authority, lifecycle, recovery, or audit requirements.

## Compatibility Personalities

Compatibility personalities translate foreign programming models into native Atarix objects, capabilities, and service requests.

A compatibility personality may emulate POSIX, firmware-style APIs, or other environments. It does not inherit the authority assumptions of those environments.

## Native Services

Native services expose Atarix semantics directly.

They participate in capability enforcement, mailbox routing, audit windows, recovery domains, and version-aware behavior.

## Hardware Services

Hardware services are accessed through explicit fabric-mediated interfaces.

Hardware acceleration may reduce latency or CPU overhead, but accelerated paths must preserve the same authority, audit, lifecycle, and recovery semantics as non-accelerated paths.
