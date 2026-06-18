# ADR-0012: Abstraction and Compatibility

Status: Accepted

Date: 2026-06-10

## Context

ATARIX is expected to evolve across multiple CPU cards, FPGA revisions, memory technologies, storage technologies, supervisor implementations, and accelerator architectures.

If higher-level software depends on implementation details, the system will become difficult to extend or replace.

## Decision

ATARIX shall define stable architectural contracts and keep implementations replaceable.

Architectural contracts include:

- Mailbox semantics.
- DMA semantics.
- Discovery records.
- Capability records.
- Memory object model.
- Supervisor service model.
- Register access semantics.
- Boot image formats.
- Test formats.

Specific implementations of those contracts may change over time.

## Evidence

The project has already separated CPU-local addressing from fabric resources, native DMA descriptors from fabric DMA descriptors, identity EEPROMs from discovery records, and discovery from authority.

Those separations reduce coupling and preserve future compatibility.

## Alternatives Considered

- Optimize every interface for the first W65C816 implementation.
- Expose FPGA internals as architectural requirements.
- Treat fixed CPU addresses as the primary system abstraction.
- Delay abstraction until a later revision.

## Consequences

Pros:

- Future CPU cards can be added.
- Future FPGA families can be used.
- Memory and storage technologies can change.
- System-level tests remain valid across implementations.
- Software can target architectural behavior rather than board-specific details.

Cons:

- More documentation is required.
- Some implementation-specific optimizations may need wrapper interfaces.
- Initial designs may be slightly more verbose.

## Future Revisions

Future ADRs may refine individual contracts, but the project should continue to prefer stable interfaces over exposed implementation details.

## Design Rule

Implementations may change.

Contracts should remain stable.