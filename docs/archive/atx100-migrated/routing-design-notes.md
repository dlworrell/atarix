# ATARIX Routing and Message Transport Design Notes

Status: Design Notes

## Purpose

This document records design considerations for message routing, dispatch, and transport optimization within ATARIX.

The primary objective is not maximum throughput. The primary objectives are:

```text
Predictable behavior
Bounded latency
Security
Capability enforcement
Observable failures
Hardware independence
```

Optimization must never compromise architectural guarantees.

## Architectural Principle

Message routing exists to deliver requests to services.

Routing does not grant authority.

Discovery does not grant authority.

Directory does not grant authority.

Visibility does not grant authority.

Capabilities grant authority.

## Mailbox Routing Goals

Mailbox routing should provide:

```text
O(1) dispatch where practical
Deterministic behavior
Bounded memory consumption
Bounded execution time
Security-first operation
Observable failure reporting
```

The mailbox layer rejects invalid traffic before service dispatch.

Recommended validation path:

```text
Header validation
Length validation
CRC validation
Sequence validation
Message type validation
Capability validation when required
Service dispatch
```

## Dispatch Tables

Preferred future implementation:

```c
handler = dispatch_table[message_type];
```

Advantages:

```text
Constant-time lookup
Simple implementation
Easy auditing
Easy testing
Suitable for firmware and FPGA translation
```

Large switch statements are acceptable early but should be reconsidered once message counts grow.

## Bitset-Based Authorization

Capabilities, permissions, allowed message classes, and fault states may be represented as bitsets where practical.

Example:

```c
if ((allowed_mask & (1u << message_type)) == 0u) {
    deny();
}
```

Advantages:

```text
Constant-time checks
Compact representation
FPGA-friendly implementation
Simple testing
```

## Sequence Windows

Mailbox sequence tracking should use bounded, constant-time validation.

Goals:

```text
Replay detection
Duplicate detection
Out-of-order detection
Gap detection
Future wraparound safety
```

Current Mailbox v1 tests cover duplicate, old, and gap rejection.

Future work should define precise wraparound semantics for 32-bit sequence numbers.

## Priority Classes

Future mailbox implementations may support priority classes.

Suggested classes:

```text
P0 Faults, watchdogs, emergency reset
P1 Security and capability operations
P2 Discovery and topology management
P3 Normal service requests
P4 Diagnostics and bulk transfers
```

Priority scheduling must not bypass validation or capability enforcement.

## Service Lookup

When service counts become large, service lookup should move from linear search to bounded lookup.

Candidates:

```text
Hash table
Perfect hash
Sorted table with binary search
FPGA lookup table
Bitset-indexed dispatch
```

Selection criteria:

```text
Deterministic timing
Predictable memory use
Auditability
Ease of simulation
Ease of hardware implementation
```

## Resource Dependency Graphs

Future ATARIX systems may model services and resources as directed graphs.

Example:

```text
Application
    -> Filesystem Service
    -> Storage Service
    -> Block Device Resource
    -> Driver Resource
    -> Hardware Implementation
```

Potential uses:

```text
Startup ordering
Shutdown ordering
Fault propagation analysis
Recovery planning
Capability reachability analysis
Resource locality optimization
Service dependency validation
```

Status: Future research.

## Multi-Node Routing

Future ATARIX deployments may include multiple Fabric nodes, CPU nodes, service cards, or accelerator cards.

Potential future techniques:

```text
Shortest-path routing
Fault-aware routing
Load-aware routing
Capability-constrained routing
Redundant-path routing
Service-locality routing
```

These techniques are not required for Mailbox Protocol v1.

## Mathematical Techniques Under Consideration

The following topics may provide future value:

```text
Graph theory
Queueing theory
Hashing theory
Bitset algebra
Finite-state machine analysis
Formal verification
Reliability modeling
Capability propagation analysis
Resource dependency graphs
Fault propagation models
```

The following are currently premature:

```text
Neural routing
Adaptive learning dispatch
Dynamic route prediction
Heuristic optimization without measurable need
```

## Optimization Rule

Optimization must never violate the required protected-access path:

```text
Application
    -> Service
    -> Capability Policy
    -> Resource
    -> Implementation
```

No routing optimization may create direct hardware access.

No routing optimization may treat Discovery or Directory visibility as authority.

All routing remains subordinate to capability validation and security policy.

## Current Recommendation

For the current virtual-first ATARIX phase, use the simplest deterministic routing model:

```text
Validate header
Validate length
Validate CRC
Validate sequence
Validate message type
Check capability if required
Dispatch by message type
Return explicit status
```

Do not introduce complex routing until a real scaling pressure exists.

## Result

These notes preserve future routing ideas without burdening Mailbox v1 with premature complexity.
