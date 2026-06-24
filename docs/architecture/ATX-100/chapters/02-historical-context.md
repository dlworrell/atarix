# Chapter 2: Historical Context

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md -->
### Integrated source: `docs/ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md`

# ADR: Discoverable, Observable, Testable, Instrumentable, Recoverable

Status: Accepted

## Context

Complex systems become difficult to maintain when subsystems cannot be located, measured, validated, diagnosed, or recovered independently.

Historically, many computer systems assumed that successful operation was sufficient evidence of correctness.

ATARIX is intended to support:

- long service life
- multiple processor architectures
- homebrew hardware
- experimental hardware
- integrated instrumentation
- self-validation
- recovery without a functioning operating system

A formal architectural doctrine is required.

## Decision

Every ATARIX subsystem shall be:

```text
Discoverable
Observable
Testable
Instrumentable
Recoverable
```

This doctrine applies to hardware, firmware, fabric services, supervisor services, and operating-system-visible services.

## Discoverable

A subsystem should be identifiable and enumerable.

Questions answered:

```text
What is it?
Where is it?
What version is it?
What services does it provide?
```

Examples:

- Identity EEPROM
- Discovery records
- Resource records
- Service records

A subsystem that cannot be discovered is difficult to manage and support.

## Observable

A subsystem should expose measurable state.

Questions answered:

```text
Is it healthy?
What is it doing?
What faults have occurred?
```

Examples:

- health state
- counters
- fault records
- telemetry
- event logs

Observation should not depend solely on the subsystem itself.

## Testable

A subsystem should support validation.

Questions answered:

```text
Does it function correctly?
Does it meet specification?
Can faults be reproduced?
```

Examples:

- validation profiles
- self-test mechanisms
- supervisor-assisted testing
- manufacturing validation

## Instrumentable

A subsystem should support measurement and analysis.

Questions answered:

```text
Can behavior be measured?
Can traffic be captured?
Can timing be characterized?
```

Examples:

- logic analyzer services
- protocol decoders
- counters
- trigger systems
- DSP analysis
- engineering test points

Instrumentation should be available independently of the operating system where practical.

## Recoverable

A subsystem should support diagnosis and restoration after faults.

Questions answered:

```text
Can it be repaired?
Can it be reprovisioned?
Can it be isolated?
Can diagnostics still operate after failure?
```

Examples:

- provisioning mode
- recovery mode
- supervisor diagnostics
- fault logging
- restricted mode operation

Recovery should not depend on successful operation of the failed subsystem.

## Relationship to Supervisor

The supervisor is the primary enforcement mechanism for this doctrine.

The supervisor should maintain visibility into platform state even when:

- CPUs are held in reset
- operating systems are unavailable
- services are degraded
- hardware faults are present

## Relationship to Instrumentation

Instrumentation is considered a first-class platform feature.

Examples:

```text
Logic Analyzer
Protocol Decoder
Pattern Generator
Counter Bank
DSP Analysis
Trigger Router
```

Instrumentation exists to support observability, testing, and recovery.

## Relationship to Enumeration

The Enumeration Fabric should make discoverable:

- resources
- services
- instrumentation endpoints
- capability descriptions
- version information

Enumeration is the primary mechanism for discoverability.

## Review Checklist

Every new specification should answer:

```text
How is it discovered?
How is it observed?
How is it tested?
How is it instrumented?
How is it recovered?
```

If one of these questions cannot be answered, the specification should justify why.

## Architectural Benefits

This doctrine:

- improves maintainability
- improves diagnosability
- improves validation quality
- improves field supportability
- supports long-term evolution
- reduces hidden assumptions
- encourages measurable engineering decisions

## Related ADRs

- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md
- ADR-THREE-FABRIC-ARCHITECTURE.md

## Guiding Principle

```text
Measurement must not depend on the thing being measured.
```

## Summary

```text
Every subsystem shall be:

Discoverable
Observable
Testable
Instrumentable
Recoverable
```

This doctrine is a core architectural requirement of ATARIX.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0001-ulx3s-selection.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0001-ulx3s-selection.md -->
### Integrated source: `docs/adr/ADR-0001-ulx3s-selection.md`

# ADR-0001: ULX3S LFE5U-85F Selection

Status: Accepted

## Context
Need a fabric controller platform.

## Decision
Use ULX3S with LFE5U-85F.

## Evidence
- Open toolchain support
- Large FPGA resource budget
- Existing community support
- Suitable for rapid prototyping

## Alternatives Considered
- Custom FPGA board
- Smaller ECP5 variants

## Consequences
Faster development and lower initial risk.

## Future Revisions
May migrate to custom ATARIX fabric controller card.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0003-local-cpu-bus.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0003-local-cpu-bus.md -->
### Integrated source: `docs/adr/ADR-0003-local-cpu-bus.md`

# ADR-0003: Local CPU Bus

Status: Accepted

## Context
Should the W65C816 bus be exposed across the backplane?

## Decision
No. The CPU bus remains local to the CPU card.

## Evidence
- Improved signal integrity
- Better scalability
- Easier SMP evolution
- Cleaner DMA architecture

## Alternatives Considered
- Shared raw bus
- VME-style architecture
- Direct backplane extension

## Consequences
Requires bridge logic and fabric interfaces.

## Future Revisions
Revisit only if evidence supports a shared-bus design.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0009-observability-first.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0009-observability-first.md -->
### Integrated source: `docs/adr/ADR-0009-observability-first.md`

# ADR-0009: Observability First

Status: Accepted

Date: 2026-06-10

## Context

Complex systems fail in unexpected ways. Debugging a system after failure is dramatically easier when visibility is designed into the architecture.

## Decision

Observability is a first-class architectural requirement.

Every major subsystem should expose status, counters, diagnostics, health information, fault history, and recovery information whenever practical.

## Evidence

- Faster hardware bring-up.
- Reduced debugging time.
- Better fault isolation.
- Easier performance characterization.
- Supports evidence-based engineering.

## Alternatives Considered

- Minimal diagnostics.
- Debugging only through external instruments.
- Diagnostics added later.

## Consequences

Pros:
- Easier troubleshooting.
- Better operational insight.
- Better long-term maintainability.

Cons:
- Additional design effort.
- Additional registers and counters.

## Future Revisions

May expand to include trace buffers, fabric analyzers, and automated health reporting.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0010-evidence-based-engineering.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0010-evidence-based-engineering.md -->
### Integrated source: `docs/adr/ADR-0010-evidence-based-engineering.md`

# ADR-0010: Evidence-Based Engineering

Status: Accepted

Date: 2026-06-10

## Context

Engineering decisions are often made from intuition, habit, or preference. ATARIX aims to favor measurable and reproducible evidence whenever practical.

## Decision

Adopt evidence-based engineering as a project-wide principle.

Measure first.
Decide second.
Document always.

## Evidence

Scientific and engineering disciplines consistently produce better outcomes when hypotheses, experiments, measurements, and conclusions are documented and reproducible.

## Alternatives Considered

- Opinion-driven design.
- Authority-driven design.
- Trend-driven design.

## Consequences

Pros:
- Better decisions.
- Reduced redesign risk.
- Improved reproducibility.
- Better ADR quality.

Cons:
- Additional documentation effort.
- Additional testing effort.

## Future Revisions

Experiment records and benchmark repositories should eventually become part of the project workflow.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0011-little-endian-data-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0011-little-endian-data-model.md -->
### Integrated source: `docs/adr/ADR-0011-little-endian-data-model.md`

# ADR-0011: Little-Endian Data Model

Status: Accepted

Date: 2026-06-10

## Context

ATARIX begins with a W65C816 CPU but is expected to grow into a larger fabric-based workstation architecture with memory services, accelerators, large identifiers, and long-lived persistent resources.

The project requires a consistent binary representation.

## Decision

All architecture-defined binary structures use little-endian encoding.

The architecture may use 8-bit, 16-bit, 24-bit, 32-bit, and 64-bit fields as appropriate.

CPU width is not system width.

## Evidence

- Natural fit with W65C816 memory representation.
- Simplifies firmware implementation.
- Simplifies protocol definition.
- Avoids mixed-endian ambiguity.
- Supports future 64-bit identifiers and timestamps.

## Alternatives Considered

- Big-endian architecture.
- Mixed-endian structures.
- 16-bit-only architecture.
- 32-bit-only architecture.
- 64-bit-only architecture.

## Consequences

Pros:
- Consistent binary representation.
- Future-proof identifiers and timestamps.
- Easier protocol evolution.

Cons:
- Requires explicit rules for large values on an 8/16-bit CPU.
- Requires documentation of multi-byte register access semantics.

## Future Revisions

Future formats may define 128-bit identifiers if required, but the little-endian rule should remain stable.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/README.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/README.md -->
### Integrated source: `docs/adr/README.md`

# Architecture Decision Records

ADR documents capture significant architectural decisions.

## Required Sections

- Title
- Status
- Date
- Context
- Decision
- Evidence
- Alternatives Considered
- Consequences
- Future Revisions

## Engineering Rule

Major architectural decisions should be supported by evidence whenever practical.

Measure first. Decide second. Document always.

## Numbering

ADR-0001
ADR-0002
ADR-0003
...
