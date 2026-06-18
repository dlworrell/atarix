# GitHub Project Organization v1

Status: Accepted

## Purpose

Define the organizational structure used to manage ATARIX development within GitHub.

Project management exists to support engineering work. The repository favors simplicity, traceability, and low administrative overhead.

## Workflow

All work follows:

```text
Discussion
    -> ADR
    -> Issue
    -> Implementation
    -> Test
    -> CI Green
    -> Close Issue
```

Design discussions belong in GitHub Discussions.

Authoritative architectural decisions belong in ADR documents.

Executable work belongs in GitHub Issues.

## Discussions

Discussions are divided into announcements and design discussions.

### Announcements

Announcements contain authoritative project information.

Current announcement numbering:

```text
001 - Project Roadmap
002 - Architecture Overview
003 - Development Workflow
```

Future announcements should reserve numbering gaps to allow insertion without renumbering.

Recommended numbering:

```text
001-099 Project governance and project status
```

### Design Discussions

Discussions are used for design exploration and architectural debate.

Recommended numbering:

```text
100-199 Architecture
200-299 Hardware
300-399 Toolchain
400-499 Kernel and operating system
```

Examples:

```text
110 - Capability Delegation Semantics
120 - Service Discovery Evolution
200 - CPU Card Mechanical Envelope
300 - 65C816 Toolchain Strategy
400 - Kernel ABI Model
```

## ADR Relationship

Discussions may lead to ADRs.

Example:

```text
Discussion 110
    -> ADR-002
    -> Issue
    -> Implementation
    -> Test
    -> CI Green
```

ADRs remain the authoritative record of architectural decisions.

## Milestones

Milestones represent sprint-level deliverables.

Current milestones:

```text
Discovery Sprint 1
Capability Sprint 1
Directory Sprint 1
Supervisor Runtime Sprint 1
Toolchain Sprint 1
65C816 Bring-up Sprint 1
```

Guideline:

```text
Milestone = weeks of work
Issue     = days of work
Task      = hours of work
```

## Issues

Issues represent actionable work.

Issues should normally have:

```text
Priority label
Type label
Area label
Milestone
```

Issue titles should describe the concrete work to be completed.

Examples:

```text
Implement capability_policy.c
Add capability evaluator tests
Implement directory_builder.c
Add supervisor health-state tests
```

## Labels

### Priority Labels

```text
P0-Critical
P1-High
P2-Normal
P3-Low
```

Priority meaning:

```text
P0 = Fix immediately
P1 = Current sprint
P2 = Future sprint
P3 = Backlog or cleanup
```

### Type Labels

```text
type:architecture
type:implementation
type:test
type:documentation
type:hardware
type:toolchain
type:ci
```

### Area Labels

```text
area:discovery
area:capability
area:directory
area:supervisor
area:health
area:kernel816
area:tooling
area:documentation
area:ci
```

## Project Board

Project:

```text
ATARIX Development
```

Status values:

```text
Backlog
Ready
In Progress
Review
Blocked
Done
```

The project board tracks workflow state. Milestones track sprint membership. Labels track priority, type, and subsystem.

## Wiki

The wiki is a navigation layer, not the source of truth.

The repository remains authoritative for:

```text
Specifications
ADRs
Headers
Source code
Tests
```

The wiki may summarize project status, link to ADRs, and provide navigation for contributors.

## Repository Governance

All code merged into main should leave CI passing.

Discovery Sprint 1 established the baseline CI process:

```text
bootstrap
configure
build
test
```

Future sprints should maintain a green build and test state.

## Revision History

```text
v1 - Initial project organization structure.
```
