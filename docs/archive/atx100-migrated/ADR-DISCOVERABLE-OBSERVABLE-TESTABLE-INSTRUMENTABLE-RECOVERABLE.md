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
