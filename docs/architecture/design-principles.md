# Atarix Design Principles

## Purpose

This document defines the guiding principles that govern all Atarix architectural and implementation decisions.

## Principle 1: Objects First

Everything visible within Atarix is represented as an object.

Services, devices, mailboxes, storage entities, fabric nodes, and kernel facilities are all expressed through a common object model.

## Principle 2: Human Names First

Public interfaces expose human-readable names.

Implementation details such as table indexes, memory addresses, handles, and slot numbers are internal implementation concerns and shall not be exposed as primary user-facing identifiers.

## Principle 3: Authority Is Explicit

Every operation must have a traceable source of authority.

Authority must never arise from undocumented special cases.

Delegation, ownership, approval, and revocation shall be explicit.

## Principle 4: Layered Security

Security responsibilities are separated into layers.

Ring boundaries provide coarse-grained isolation.

Capabilities provide fine-grained authority.

Mailboxes provide communication transport and do not independently grant authority.

## Principle 5: Distribution Is Native

Architectural decisions shall assume that objects may eventually exist on remote nodes.

Subsystems should avoid assumptions that require all objects to reside locally.

## Principle 6: Hardware Independence

Core architecture shall not depend upon:

- A specific processor family
- A memory management unit
- A specific address size
- A specific bus architecture

The architecture must remain portable across simulation, FPGA, and physical hardware implementations.

## Principle 7: Testability First

Every subsystem shall be testable in simulation.

Architectural correctness must be verifiable through automated tests.

Security and boundary enforcement shall always have corresponding regression tests.

## Principle 8: Architecture Before Implementation

Architecture defines implementation.

Implementation shall not define architecture.

When architectural uncertainty exists, documentation and review take precedence over coding.
