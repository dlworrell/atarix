# ATARIX

ATARIX is a modular experimental workstation and operating-system research architecture built around interchangeable CPU cards, a Fabric Northbridge implemented in an ECP5 FPGA, a supervisor management plane, capability-mediated resource access, explicit security doctrine, and a Sun/NuBus/UPA-inspired modular backplane.

The initial Rev A CPU-card target is the WDC W65C816S. That processor is an implementation target, not a permanent architectural constraint. ATARIX is designed so that future CPU cards can be introduced without changing the public system architecture.

## Current Documentation Model

The repository is being migrated from many individual Markdown source documents into a canonical ATX-100 architecture book.

Canonical architecture material now lives under:

- [ATX-100 Compiled Architecture Book](docs/architecture/ATX-100/ATX-100-COMPILED.md)
- [ATX-100 Chapters](docs/architecture/ATX-100/chapters/)
- [Migrated Source Archive](docs/archive/atx100-migrated/)

The migrated archive preserves the original source documents for provenance. The chapter files and compiled book are the working canonical architecture outputs.

## Core Doctrine

ATARIX follows a trust-nothing architecture.

Core principles:

```text
Authority is explicit.
Trust is explicit.
Ownership is explicit.
Persistence is explicit.
Cleanup is explicit.
System objects are not exempt.
Identity is not authority.
Connectivity is not trust.
Compromise is expected.
Containment is mandatory.
```

The project treats architecture as the product and code as an implementation of that architecture.

## Architecture Direction

ATARIX is not a raw W65C816 bus stretched across a backplane.

The current design direction is:

```text
CPU Card
    -> CPU-local SRAM and ROM
    -> Fabric Northbridge Interface
    -> DIN41612-style modular backplane
    -> ECP5 Fabric Northbridge
    -> Service-oriented architecture
    -> Memory cards, accelerators, networking, and future CPU cards
```

The supervisor management plane is separate from the main control/data/event fabric and remains responsible for reset, recovery, RTC, watchdogs, health monitoring, and fault logging.

## ATX-100 Chapter Set

The ATX-100 book is organized as a 22-chapter architecture manual:

- [Chapter 1: Purpose and Scope](docs/architecture/ATX-100/chapters/01-purpose-and-scope.md)
- [Chapter 2: Historical Context](docs/architecture/ATX-100/chapters/02-historical-context.md)
- [Chapter 3: Documentation Taxonomy](docs/architecture/ATX-100/chapters/03-documentation-taxonomy.md)
- [Chapter 4: Architecture Before Implementation](docs/architecture/ATX-100/chapters/04-architecture-before-implementation.md)
- [Chapter 5: Security Before Convenience](docs/architecture/ATX-100/chapters/05-security-before-convenience.md)
- [Chapter 6: Human Readability](docs/architecture/ATX-100/chapters/06-human-readability.md)
- [Chapter 7: Fabric-Centric Computing](docs/architecture/ATX-100/chapters/07-fabric-centric-computing.md)
- [Chapter 8: Engineering as Architecture](docs/architecture/ATX-100/chapters/08-engineering-as-architecture.md)
- [Chapter 10: Object Model](docs/architecture/ATX-100/chapters/10-object-model.md)
- [Chapter 11: Capability Model](docs/architecture/ATX-100/chapters/11-capability-model.md)
- [Chapter 12: Mailbox Transport](docs/architecture/ATX-100/chapters/12-mailbox-transport.md)
- [Chapter 13: Directory and Discovery Services](docs/architecture/ATX-100/chapters/13-directory-and-discovery-services.md)
- [Chapter 14: Memory and Data Movement](docs/architecture/ATX-100/chapters/14-memory-and-data-movement.md)
- [Chapter 15: Lookup Acceleration](docs/architecture/ATX-100/chapters/15-lookup-acceleration.md)
- [Chapter 16: Processor Modules](docs/architecture/ATX-100/chapters/16-processor-modules.md)
- [Chapter 17: Fabric Northbridge](docs/architecture/ATX-100/chapters/17-fabric-northbridge.md)
- [Chapter 18: Supervisor Controller](docs/architecture/ATX-100/chapters/18-supervisor-controller.md)
- [Chapter 19: Storage and Networking](docs/architecture/ATX-100/chapters/19-storage-and-networking.md)
- [Chapter 20: AEMS and Engineering Gates](docs/architecture/ATX-100/chapters/20-aems-and-engineering-gates.md)
- [Chapter 21: Requirements and Traceability](docs/architecture/ATX-100/chapters/21-requirements-and-traceability.md)
- [Chapter 22: Roadmap and Evolution](docs/architecture/ATX-100/chapters/22-roadmap-and-evolution.md)

Chapter 9 is being reconciled as part of the migration cleanup.

## Build Commands

Standard documentation commands:

```sh
make docs-plan
make docs-book
make docs-migrate
```

`make docs-book` builds normal generated documentation outputs and migration plans.

`make docs-migrate` applies the migration, integrates source documents into ATX-100 chapters, writes the compiled book, and moves consumed source files into the archive.

## AEMS Relationship

AEMS is the engineering management system used to govern the ATARIX project. ATARIX consumes AEMS concepts, but AEMS is expected to become a standalone repository and standard.

See the repository split plan:

- [Repository Extraction Plan](docs/roadmap/repository-extraction-plan.md)

## Repository Direction

The project is being organized into separate repositories:

```text
aems          Engineering standard and reusable tools
atarix        Modular workstation architecture and implementation
jag           AI-assisted engineering assistant work
just-a-geek   Public company, website, brand, and publication material
```

ATARIX remains the architecture and implementation repository.

AEMS should remain independent.

JAG and Just a Geek should consume or publish outputs from AEMS and ATARIX without owning their canonical source material.

## Current Status

The current priority is repository stabilization:

```text
1. Complete ATX-100 documentation migration.
2. Keep all migrated source material archived and traceable.
3. Repair Markdown links.
4. Get CI green.
5. Extract AEMS only after ATARIX documentation is stable.
```

The goal is a repository that can be cloned years from now and still explain what ATARIX is, why it was designed this way, and where every major decision came from.
