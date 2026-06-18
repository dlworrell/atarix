# Chapter 5: Security Before Convenience

## AEMS Integrated Material

<!-- AEMS-MIGRATED-SOURCE: docs/ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md -->
### Integrated source: `docs/ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md`

# ADR: Identity, Trust, and Capability Separation

Status: Accepted

## Context

Many systems implicitly treat identification as authorization.

Example:

```text
Identity
    => Authority
```

This creates tight coupling between discovery, provisioning, security, and resource access.

ATARIX is intended to support:

- factory hardware
- development hardware
- prototype hardware
- homebrew hardware
- future third-party hardware

without requiring changes to core platform architecture.

## Decision

ATARIX separates three concepts:

```text
Identity proves existence.
Trust determines authority.
Capabilities determine access.
```

### Identity

Identity answers:

```text
Who are you?
What are you?
Who built you?
```

Identity is provided by the Identity EEPROM.

Identity validation proves only that a board can identify itself and that the identity record is valid.

Identity alone grants no authority.

### Trust

Trust answers:

```text
How much confidence should the platform place in this device?
```

Trust is evaluated by supervisor policy.

Trust may be influenced by:

- provisioning state
- board class
- trust class
- development mode
- manufacturing records
- future cryptographic validation

Trust does not automatically grant access to resources.

### Capabilities

Capabilities answers:

```text
What may this device do?
```

Capabilities describe permissions and allowed operations.

Examples:

- DMA access
- boot authority
- recovery authority
- mailbox participation
- interrupt generation
- service publication

Capabilities are evaluated independently of identity.

## Consequences

The following states are legal:

```text
Identified but Untrusted
Trusted but Restricted
Homebrew but Allowed
Factory but Limited
```

This supports safe extensibility while preserving platform control.

## Architectural Benefits

- Supports homebrew cards without implicit trust.
- Supports future security enhancements.
- Reduces coupling between discovery and authorization.
- Allows policy evolution without changing hardware identity formats.
- Keeps provisioning, discovery, and capability systems independent.

## Related Specifications

- identity-eeprom-v1.md
- discovery-rom-format-v1.md
- capability-record-format-v1.md
- service-directory-format-v1.md
- factory-provisioning-v1.md

## Related Principles

```text
Identity Is A First-Class Service
Observability Is A Feature
Provisionability Is A Feature
Recovery Is Mandatory
```

<!-- AEMS-MIGRATED-SOURCE: docs/ADR-RING-SECURITY-MODEL.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ADR-RING-SECURITY-MODEL.md -->
### Integrated source: `docs/ADR-RING-SECURITY-MODEL.md`

# ADR: Ring Security Model

Status: Accepted

## Context

ATARIX separates identity, enumeration, trust, capabilities, directory services, and transport.

That separation prevents discovery from becoming authorization, but the platform still needs a clear privilege model that explains which agents may validate, enumerate, authorize, configure, and operate system resources.

The ring security model provides that privilege structure.

## Decision

ATARIX uses a ring-style security model across firmware, supervisor services, fabric services, operating system services, and applications.

The ring model applies to:

- Management Fabric
- Enumeration Fabric
- Transport Fabric
- Identity EEPROM validation
- Discovery ROM enumeration
- Capability Records
- Service Directory
- Instrumentation services
- Provisioning and recovery
- CPU cards and future SMP systems

## Core Rule

```text
Enumeration describes.
Capabilities authorize.
Rings constrain authority.
```

A discovered service is not automatically usable.

A valid capability is not automatically usable from every privilege level.

## Proposed Rings

```text
Ring -2  Hardware Safety / Supervisor Authority
Ring -1  Fabric / Platform Firmware Authority
Ring  0  Kernel / Executive Authority
Ring  1  Driver / Service Authority
Ring  2  User Service Authority
Ring  3  Application Authority
```

Negative rings are platform authority rings outside the normal CPU operating-system privilege model.

## Ring -2: Hardware Safety / Supervisor Authority

Primary authority:

```text
RP2350 Supervisor
```

Responsibilities:

- safety policy
- reset control
- health monitoring
- identity validation
- provisioning authorization
- recovery entry
- fault logging
- card power/health policy
- engineering console access

Ring -2 must remain useful when CPUs, operating systems, or transport services are unavailable.

## Ring -1: Fabric / Platform Firmware Authority

Primary authorities:

```text
ECP5 Fabric Controller
Backplane BIOS
Platform firmware
```

Responsibilities:

- enumeration support
- fabric configuration
- transport setup
- service binding support
- instrumentation routing
- early boot services
- platform discovery handoff

Ring -1 is below the operating system but above ordinary device consumers.

## Ring 0: Kernel / Executive Authority

Primary authority:

```text
Operating system kernel
```

Responsibilities:

- process management
- memory management
- driver loading
- service policy consumption
- transport use
- runtime capability enforcement

Ring 0 consumes platform services but does not supersede supervisor safety authority.

## Ring 1: Driver / Service Authority

Responsibilities:

- device drivers
- privileged runtime services
- kernel-adjacent daemons
- hardware-facing software services

Ring 1 components may receive delegated capabilities.

## Ring 2: User Service Authority

Responsibilities:

- user-space services
- network daemons
- filesystem services
- desktop services
- application frameworks

Ring 2 components operate with explicit service capabilities.

## Ring 3: Application Authority

Responsibilities:

- ordinary applications
- scripts
- user tools

Ring 3 has no direct hardware authority unless explicitly mediated by higher rings.

## Relationship to Identity

Identity EEPROM validation occurs under Ring -2 authority.

Identity proves existence.

Identity does not grant operational authority.

## Relationship to Discovery ROM

Discovery ROM enumeration occurs under Ring -1 / Ring -2 observation depending on boot phase.

Discovery describes resources and services.

Discovery does not grant access.

## Relationship to Capabilities

Capabilities are interpreted in the context of ring authority.

A capability record should identify:

- operation
- target resource or service
- allowed ring or minimum ring
- delegation rules
- revocation rules where applicable

A capability valid for Ring -1 is not automatically valid for Ring 0 or Ring 3.

## Relationship to Directory Services

Directory services may inform policy and binding.

Directory services do not override Ring -2 safety authority.

Recovery must not depend on directory services.

## Relationship to Instrumentation

Instrumentation services may expose highly sensitive platform visibility.

Passive observation may be available to lower rings by policy.

Active stimulus, pattern generation, trigger routing, and destructive tests require high authority.

Recommended default:

```text
Passive counters: Ring 0 or delegated Ring 1/2
Trace capture: Ring 0 or delegated service
Pattern generation: Ring -2 / Ring -1 only unless explicitly authorized
Recovery instrumentation: Ring -2
```

## Relationship to Homebrew Cards

Homebrew cards may be identified and enumerated without being trusted.

Default policy:

```text
Homebrew identity: allowed to enumerate if valid
Homebrew services: restricted by capability and ring policy
Homebrew DMA: denied unless explicitly authorized
Homebrew boot authority: denied unless explicitly authorized
Homebrew supervisor authority: denied
```

## Boot-Time Authority Chain

Recommended boot authority flow:

```text
Ring -2 Supervisor validates platform health and identity
    ↓
Ring -1 Platform firmware enumerates resources and services
    ↓
Capabilities are evaluated against trust and policy
    ↓
Ring 0 kernel receives authorized platform view
    ↓
Ring 1/2/3 receive delegated services
```

## Security Review Checklist

Every new specification should answer:

```text
What ring owns this function?
What ring may observe it?
What ring may configure it?
What ring may use it?
What ring may delegate it?
What ring may recover it?
```

## Related ADRs

- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md
- ADR-THREE-FABRIC-ARCHITECTURE.md
- ADR-DISCOVERABLE-OBSERVABLE-TESTABLE-INSTRUMENTABLE-RECOVERABLE.md

## Related Specifications

- identity-eeprom-v1.md
- discovery-rom-format-v1.md
- supervisor-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- cpu-observability-contract-v1.md
- future capability-record-format-v1.md
- future service-directory-format-v1.md

## Summary

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Rings constrain authority.
Transport operates.
```

<!-- AEMS-MIGRATED-SOURCE: docs/ADR-THREE-FABRIC-ARCHITECTURE.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ADR-THREE-FABRIC-ARCHITECTURE.md -->
### Integrated source: `docs/ADR-THREE-FABRIC-ARCHITECTURE.md`

# ADR: Three Fabric Architecture

Status: Accepted

## Context

Traditional systems often combine management, enumeration, and data transport into a single bus architecture.

This creates coupling between:

- platform health
- device discovery
- resource authorization
- bulk data movement
- recovery operations

ATARIX is intended to support multiple processor architectures, long service life, integrated instrumentation, self-validation, and recovery independent of the operating system.

A more explicit separation of concerns is required.

## Decision

ATARIX adopts a Three Fabric Architecture.

```text
Management Fabric
Enumeration Fabric
Transport Fabric
```

Each fabric has distinct responsibilities and authority.

## Management Fabric

The Management Fabric is governed by the supervisor.

Primary authority:

```text
RP2350 Supervisor
```

Responsibilities include:

```text
Identity validation
Health monitoring
Telemetry
Provisioning
Recovery
Reset control
Fault logging
Policy enforcement
Instrumentation management
```

Primary question answered:

```text
Is the platform healthy and allowed to operate?
```

The Management Fabric remains available as much as practical during failure and recovery scenarios.

## Enumeration Fabric

The Enumeration Fabric describes the platform.

Primary authority:

```text
Fabric Controller
```

Responsibilities include:

```text
Identity records
Discovery records
Resource records
Service records
Capability descriptions
Provider descriptions
Version descriptions
Binding information
```

Primary questions answered:

```text
What exists?
What services exist?
What versions exist?
How do I bind to them?
```

The Enumeration Fabric describes resources and services.

It does not grant authority.

## Transport Fabric

The Transport Fabric moves information.

Responsibilities include:

```text
Mailbox traffic
DMA traffic
Interrupt signaling
Shared memory transport
Network transport
Bulk data movement
```

Primary question answered:

```text
How do I use the platform?
```

The Transport Fabric is optimized for operation rather than description or governance.

## Relationship Between Fabrics

The fabrics operate together but remain logically distinct.

```text
Management Fabric
    validates
    monitors
    governs

Enumeration Fabric
    identifies
    describes
    binds

Transport Fabric
    moves
    signals
    exchanges
```

## Boot Sequence Model

A typical startup flow is:

```text
Power On
    ↓
Supervisor Initialization
    ↓
Identity Validation
    ↓
Enumeration
    ↓
Capability Evaluation
    ↓
Service Binding
    ↓
Transport Activation
    ↓
Monitor
    ↓
Operating System
```

## Relationship to Identity, Trust, and Capability

The Three Fabric Architecture works together with the Identity/Trust/Capability model.

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Transport operates.
```

Trust evaluation belongs to Management Fabric policy.

Resource and service description belong to the Enumeration Fabric.

Operational communication belongs to the Transport Fabric.

## Instrumentation Integration

Instrumentation services span all three fabrics.

Examples:

```text
Logic Analyzer
Protocol Decoder
Pattern Generator
Counter Bank
DSP Analysis
Trigger Router
```

Instrumentation remains under Management Fabric authority even when exposed through Enumeration and Transport mechanisms.

## Architectural Benefits

This separation:

- reduces coupling
- improves recovery behavior
- improves observability
- supports multiple CPU architectures
- simplifies future expansion
- supports homebrew and experimental hardware
- supports integrated testbench functionality

## Consequences

A CPU card does not require direct knowledge of:

- slot numbering
- physical wiring
- transport implementation details
- fabric topology details

The platform exposes services and resources through enumeration and transport abstractions.

## Related ADRs

- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md

## Related Specifications

- identity-eeprom-v1.md
- supervisor-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- discovery-rom-format-v1.md
- capability-record-format-v1.md
- service-directory-format-v1.md

## Guiding Principle

```text
Management Fabric
    validates and governs

Enumeration Fabric
    describes and binds

Transport Fabric
    transfers and signals
```

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0002-rp2350-selection.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0002-rp2350-selection.md -->
### Integrated source: `docs/adr/ADR-0002-rp2350-selection.md`

# ADR-0002: RP2350 Supervisor Selection

Status: Accepted

Date: 2026-06-10

## Context

ATARIX needs an always-available supervisor controller for reset sequencing, watchdogs, RTC coordination, fault logging, recovery-mode control, power monitoring, and management-plane communication.

The supervisor must remain operational when the W65C816 CPU card or the FPGA fabric controller is not booting.

## Decision

Use the RP2350 as the Rev A supervisor MCU target.

## Evidence

- Strong GPIO, timer, and peripheral support.
- More headroom than RP2040 for watchdog policy, anomaly scoring, management commands, and future cryptographic checks.
- Good toolchain availability.
- Good community and documentation ecosystem.
- Suitable for deterministic low-speed management functions.

## Alternatives Considered

- RP2040.
- STM32 family.
- ESP32-S3.
- Small CPLD-only supervisor.
- Supervisor implemented inside the FPGA fabric.

## Consequences

Pros:

- Keeps management independent of the main CPU and FPGA fabric.
- Gives the supervisor enough compute headroom for health scoring and fault handling.
- Keeps Rev A implementation accessible and inexpensive.

Cons:

- Adds another firmware target.
- Requires explicit management-plane protocol design.
- Requires careful authority separation between supervisor and fabric controller.

## Future Revisions

Future boards may replace the RP2350, but the logical supervisor role should remain stable.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-0014-testing-and-verification-strategy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-0014-testing-and-verification-strategy.md -->
### Integrated source: `docs/adr/ADR-0014-testing-and-verification-strategy.md`

# ADR-0014: Testing and Verification Strategy

Status: Accepted

Date: 2026-06-10

## Context

ATARIX combines hardware, firmware, FPGA logic, protocols, security mechanisms, and operating-system research.

Verification must be treated as a required engineering activity.

## Decision

Testing and verification are first-class architectural requirements.

All major architectural interfaces should have documented tests, validation procedures, or simulation coverage.

The project testing authority is:

```text
docs/testing-strategy.md
```

## Principles

- Test behavior rather than implementation details.
- Preserve reproducibility.
- Version test formats.
- Prefer small targeted tests.
- Support randomized testing where practical.
- Record sufficient state to diagnose failures.
- Keep architectural tests portable across implementations.

## Scope

Testing applies to:

- CPU behavior.
- Firmware.
- FPGA fabric.
- Discovery records.
- Mailbox protocols.
- DMA descriptors.
- Capability records.
- Supervisor services.
- Recovery mechanisms.
- Hardware bring-up.
- Integration workflows.

## Consequences

Pros:

- Earlier defect discovery.
- Better documentation.
- Easier hardware bring-up.
- Safer architectural evolution.

Cons:

- Additional engineering effort.
- Additional tooling.
- Additional CI infrastructure.

## Design Rule

If a subsystem cannot be verified, it is not complete.

Observability and testability are architectural features.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-00X-capability-evaluation-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-00X-capability-evaluation-model.md -->
### Integrated source: `docs/adr/ADR-00X-capability-evaluation-model.md`

# ADR-00X: Capability Evaluation Model

Status: Accepted

## Context

ATARIX uses a layered security model built around identity, trust, rings, capabilities, resources, and operations.

The Capability Engine must not be an independent permission system. It must implement the ATARIX trust and ring model.

The security chain is:

```text
Identity
    -> Trust
    -> Revocation
    -> Ring
    -> Ownership
    -> Capability
    -> Resource
    -> Operation
    -> ALLOW / DENY
```

## Decision

ATARIX capability evaluation shall be based on the following authority model:

```text
Supervisor (-2)
    Root of Trust

Fabric (-1)
    Enforcement Authority

Kernel (0)
    System Authority

Drivers (1)
    Device Authority

Services (2)
    Service Authority

Applications (3)
    User Authority
```

The Supervisor is the root of trust.

The Fabric is the enforcement authority.

Capabilities are bound to principals.

Capabilities are not transferable.

Delegation is permitted only when authority is reduced.

Revocation is hybrid: capabilities may expire naturally, and the Supervisor may revoke them immediately.

The default decision is DENY.

## Root of Trust

The Supervisor operates at Ring -2 and is the root of trust.

Responsibilities include:

```text
Identity management
Trust establishment
Trust revocation
Health monitoring
Recovery coordination
```

No entity below Ring -2 may establish system trust.

## Enforcement Authority

The Fabric operates at Ring -1 and enforces access decisions.

Responsibilities include:

```text
Capability enforcement
Resource mediation
Transport authorization
Ring boundary enforcement
```

A CPU, service, driver, or application may request access, but the Fabric is authoritative for fabric-mediated access.

## Principal-Bound Capabilities

Capabilities are bound to a principal.

Possession does not imply ownership.

A capability issued to one principal may not be used by another principal.

A capability request must therefore identify:

```text
Caller principal
Capability owner principal
Issuer principal
Target resource
Requested operation
```

The evaluator must verify that the caller is allowed to use the capability before evaluating the operation.

## Non-Transferability

Capabilities are non-transferable by default and by design.

A principal may not hand an existing capability to another principal as a way to grant authority.

If authority is to be delegated, the system must create a separate delegated capability with reduced authority and its own identity.

## Delegation

Delegation may reduce authority only.

Delegation may never:

```text
Increase ring privilege
Add operations
Expand resource scope
Bypass trust requirements
Bypass revocation
Create Supervisor authority
```

A delegated capability must remain traceable to its parent capability.

## Revocation

ATARIX uses a hybrid revocation model.

Capabilities may have expiration times.

Capabilities may also be revoked immediately by the Supervisor.

A capability is usable only if:

```text
Capability is structurally valid
Capability is not expired
Capability is not revoked
Issuer remains trusted
Owner remains trusted
```

Revocation is authoritative and immediate for future access attempts once the Fabric has received the revocation state.

## Evaluation Order

Capability evaluation shall proceed in the following order:

```text
1. Validate request structure
2. Validate caller identity
3. Validate issuer trust
4. Validate owner trust
5. Check revocation state
6. Check expiration
7. Check caller ring
8. Check capability ownership
9. Check delegation chain
10. Check resource scope
11. Check requested operation
12. Return ALLOW or DENY
```

The evaluator shall stop at the first failing condition and return DENY or a more specific restricted decision where applicable.

## Security Invariants

The following invariants define the model:

```text
I1  Trust precedes capability evaluation.
I2  Capabilities never increase authority.
I3  Delegation may only reduce authority.
I4  Ring boundaries are absolute.
I5  Supervisor authority is non-delegable.
I6  Fabric enforcement is authoritative.
I7  Deny-by-default.
I8  Capabilities are principal-bound.
I9  Revocation is authoritative and immediate.
```

## Ring Enforcement

A capability may not allow a caller to act above its authorized ring.

Ring boundaries are absolute.

A lower-authority principal cannot use a capability to become a higher-authority principal.

Examples:

```text
Application -> Kernel authority: DENY
Service -> Supervisor authority: DENY
Driver -> Fabric authority: DENY
```

## Supervisor Authority

Supervisor authority is non-delegable.

Operations such as trust establishment, trust revocation, power-cycle authority, and recovery override are Supervisor functions.

A capability may describe access to Supervisor-mediated services, but it may not transfer Supervisor authority to another ring.

## Fabric Enforcement

The Fabric is responsible for enforcing policy for fabric-mediated resources and transports.

The Fabric evaluates:

```text
Identity
Trust
Revocation
Ring
Ownership
Capability
Resource
Operation
```

before allowing access.

## Deny-by-Default

If any required field, trust state, revocation state, ownership relation, ring relation, delegation relation, resource scope, or operation permission is missing or invalid, the result is DENY.

ALLOW requires explicit success through the complete evaluation path.

## Consequences

Benefits:

```text
Clear trust root
Fabric-mediated enforcement
Principal-bound authority
Reduced confused-deputy risk
Immediate revocation support
Deterministic evaluation order
Testable security invariants
```

Costs:

```text
More metadata required per request
Delegation requires explicit derived capabilities
Fabric must receive revocation state
Policy evaluation is stricter than simple ACL checks
```

## Implementation Direction

Capability Sprint 1 shall implement this model through:

```text
include/atarix/capability_policy.h
src/capability/capability_policy.c
tests/capability/
```

Initial tests should cover:

```text
Capability validation
Ownership enforcement
Ring boundary enforcement
Delegation reduction
Revocation denial
Expiration denial
Operation allow/deny
Deny-by-default behavior
```

## Related Documents

```text
docs/trust-model-v1.md
docs/capability-record-format-v1.md
docs/operation-id-registry-v1.md
docs/service-directory-format-v1.md
include/atarix/rings.h
include/atarix/capability.h
```

## Revision History

```text
v1 - Initial accepted capability evaluation model.
```

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-00Y-service-oriented-resource-access.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-00Y-service-oriented-resource-access.md -->
### Integrated source: `docs/adr/ADR-00Y-service-oriented-resource-access.md`

# ADR-00Y: Service-Oriented Resource Access

Status: Accepted

## Context

ATARIX is built around a Fabric-mediated architecture, a Supervisor-owned trust model, Discovery, Directory, and capability-based authorization.

As hardware and software subsystems are added, the project must avoid drifting into a conventional model where software directly reaches into physical addresses, device registers, or backplane resources.

Direct access would undermine the security model, make hardware implementation details visible as software contracts, and create bypass paths around capability policy.

## Decision

ATARIX is service-addressed, not address-addressed.

All hardware and protected resources are accessed through named services, discovered through Discovery, resolved through Directory, mediated by Fabric, and authorized by Capability Policy.

No software component may treat physical addresses, raw registers, device locations, or known handles as authority.

## Core Rules

```text
No direct hardware access.
No raw address access.
No bypass around Fabric abstraction.
No bypass around Capability Policy.
```

The required access path is:

```text
Discovery
    -> Directory
    -> Service Handle
    -> Capability Policy
    -> Fabric Enforcement
    -> Resource Access
```

## Invariants

### No Direct Hardware Access

Software shall not directly access hardware resources.

All protected hardware access shall occur through Fabric-mediated services.

This applies to:

```text
Applications
Services
Drivers
Kernel subsystems
Fabric clients
```

Only the implementation of a service may know the underlying physical hardware details required to fulfill that service contract.

### Addresses Are Not Interfaces

Physical addresses are implementation details.

Service contracts are interfaces.

A UART, block device, mailbox, framebuffer, timer, or accelerator may be implemented:

```text
Inside FPGA logic
On a local CPU card
On a peripheral card
Behind a bridge
As a virtual service
On a future remote node
```

The caller shall interact with the service contract, not the physical placement.

### Discovery Is Not Authority

Discovery advertises existence.

Discovery answers:

```text
What exists?
```

Discovery does not answer:

```text
May I use it?
```

### Directory Is Not Authority

Directory describes and resolves resources.

Directory answers:

```text
Where is it?
How is it named?
What service contract does it expose?
```

Directory does not answer:

```text
May I use it?
```

### Knowledge Is Not Authority

The following do not grant authority:

```text
Knowing a physical address
Knowing a register offset
Knowing a handle
Knowing a service name
Enumerating Discovery
Resolving Directory
```

Only successful capability evaluation grants authority.

### Fabric Is the Enforcement Point

Supervisor owns trust.

Fabric enforces trust.

Every protected operation must ultimately pass through:

```text
Capability Policy
    -> Fabric Enforcement
    -> Resource Access
```

## Consequences

This decision intentionally rejects code patterns such as:

```c
#define UART_BASE 0xF0000000u
*(volatile unsigned int *)(UART_BASE + 0x10u) = value;
```

ATARIX callers should instead use service-oriented access patterns:

```text
lookup service
obtain handle
present capability
submit request
receive response
```

The exact C API for this flow will be defined by later Directory and service-call sprints.

## Security Consequences

Possession of an address is not authority.

Possession of a handle is not authority.

Directory lookup is not authority.

Discovery enumeration is not authority.

Only a valid capability, evaluated by policy and enforced by Fabric, authorizes action.

## Hardware Validation Requirements

Hardware Validation Sprint 1 and later bench tests shall prove that:

```text
Discovery only        -> no authority
Directory only        -> no authority
Known handle only     -> no authority
Known address only    -> no authority
Valid capability      -> evaluate through policy
Invalid capability    -> deny
Revoked capability    -> deny
Expired capability    -> deny
```

Bench validation should include attempts to bypass the Fabric service path and confirm that those bypasses fail.

## Relationship to Other ADRs

This ADR depends on the Capability Evaluation Model.

It will constrain the Directory Model, Fabric Service Model, Memory Services, DMA, Storage, Networking, GPU services, and SMP integration.

## Result

ATARIX resources are accessed as services, not raw addresses.

The system architecture preserves abstraction, portability, and capability-mediated security by prohibiting direct hardware access as a software contract.

<!-- AEMS-MIGRATED-SOURCE: docs/adr/ADR-00Z-core-architectural-doctrines.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/adr/ADR-00Z-core-architectural-doctrines.md -->
### Integrated source: `docs/adr/ADR-00Z-core-architectural-doctrines.md`

# ADR-00Z: Core Architectural Doctrines

Status: Accepted

## Context

ATARIX now has a tested Discovery baseline, a tested Capability Policy baseline, and an emerging hardware-validation path.

The project needs a compact set of architectural doctrines that future ADRs, APIs, tests, code reviews, and bench procedures can be checked against.

These doctrines are not low-level implementation rules. They are the constitutional rules of the system.

## Decision

ATARIX adopts the following core architectural doctrines.

## Doctrines

### 1. CPU Width Is Not System Width

The W65C816 is a local processor, not the definition of the whole system bus.

ATARIX may use local 65C816 buses, Fabric services, DMA engines, wider memory systems, and future heterogeneous processors without exposing all resources as direct 65C816-addressable hardware.

### 2. Supervisor Owns Trust

The Supervisor is the root of trust and recovery authority.

It owns reset, watchdog, recovery, health monitoring, trust establishment, and fault response.

### 3. Fabric Enforces Trust

The Fabric is the enforcement point for protected operations.

Trust decisions and capability policy must be enforced at the Fabric boundary before resource access is granted.

### 4. Everything Is a Service

Hardware and system resources are exposed as services.

A UART, timer, storage device, framebuffer, network interface, accelerator, memory window, or diagnostic endpoint is accessed through a service contract, not by assuming a raw address.

### 5. Discovery Advertises

Discovery tells the system what exists.

Discovery does not grant authority.

### 6. Directory Describes

Directory names, resolves, and describes services.

Directory does not grant authority.

### 7. Capabilities Authorize

Capability Policy is the authority mechanism for protected resource access.

A request must be validated through capability policy before protected work is performed.

### 8. No Direct Hardware Access

Software components shall not bypass services to access protected hardware directly.

Direct register or physical-address access is not a public software contract.

### 9. Addresses Are Implementation Details

Physical addresses, register offsets, transport details, and device placement are implementation details hidden behind service contracts.

The same service may later move between FPGA logic, a CPU card, a peripheral card, or a virtual implementation without changing callers.

### 10. Deny by Default

The default authorization result is DENY.

A request is allowed only after satisfying all applicable identity, trust, ring, ownership, resource, operation, revocation, expiration, and delegation checks.

### 11. Hardware Implementation Is Not Software Contract

Rev A hardware choices must not become unnecessary permanent software assumptions.

Software should depend on service contracts and public headers, not board-specific placement.

### 12. Knowledge Is Not Authority

Knowing an address is not authority.

Knowing a handle is not authority.

Knowing a service name is not authority.

Discovery access is not authority.

Directory access is not authority.

Only successful capability evaluation grants authority.

## Architectural Review Rule

Every significant subsystem, public API, and bench test should be reviewable against these questions:

```text
Which doctrine does this implement?
Which ADR justifies this behavior?
Which public API expresses the contract?
Which test or bench procedure enforces it?
```

If the answer is unclear, the design is not yet ready.

## Enforcement

These doctrines are enforced through:

```text
Architecture Decision Records
Public headers
Unit tests
CI
Code review
Bench validation
Fault injection
```

## Relationship to Current Work

Discovery Sprint 1 supports:

```text
Discovery advertises.
```

Capability Sprint 1 supports:

```text
Capabilities authorize.
Deny by default.
Knowledge is not authority.
```

Hardware Validation Sprint 1 will begin proving:

```text
Supervisor owns trust.
Fabric enforces trust.
Everything is a service.
No direct hardware access.
```

## Consequences

Future Directory, Fabric Service, Memory Services, DMA, Storage, Networking, GPU, and SMP work must preserve these doctrines.

Designs that expose raw hardware access as the ordinary programming model violate this ADR.

Designs that allow Discovery or Directory to grant access without capability policy violate this ADR.

Designs that bypass Fabric enforcement for protected operations violate this ADR.

## Result

ATARIX has a defined architectural constitution.

Future work should extend the system without weakening these doctrines.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-001-Security-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-001-Security-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-001-Security-Model.md`

# ATX-SPEC-001 Security Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the mandatory security architecture of Atarix.

The Security Model exists to make compromise expensive, contained, observable, recoverable, and difficult to expand. It does not claim that exploitation can be made impossible.

## Core Doctrine

Atarix assumes:

- Software defects exist.
- Remote code execution may occur.
- Services may be compromised.
- User identities may be stolen.
- Internal nodes are not inherently trusted.
- Connectivity is not trust.
- Cleanup failures become security failures.
- Unknown state is expected.

Security is therefore centered on explicit authority, containment, lifecycle control, and auditability.

## Architectural Axioms

The Security Model depends on these axioms:

```text
Name is not identity.
Identity is not authority.
Lookup is not access.
Observation is not control.
Execution is not authority.
Ownership is not authority.
Allocation is not ownership.
Persistence is not ownership.
Connectivity is not trust.
Bootstrap authority is not runtime authority.
Unknown state must be explicit.
When authority cannot be verified, fail closed.
```

## Authority Rule

Code execution never creates authority.

A process, object, service, driver, mailbox endpoint, CPU card, fabric node, or compatibility environment may only perform operations for which it has explicit authority.

Authority may be derived from:

```text
Ownership policy
Capabilities
Delegation
Ring policy
Supervisor-mediated recovery authority
Bootstrap authority during boot only
Administrative policy
```

Authority may not be derived from:

```text
Identity alone
User name alone
Object name alone
Directory lookup
Physical location
Network reachability
Execution context
System component status
```

## Security Evaluation

A normal runtime access decision is constrained by:

```text
Object identity
  -> Ring policy
  -> Ownership / authority policy
  -> Capability validation
  -> Revocation state
  -> Expiration state
  -> Lifecycle state
  -> Resource policy
  -> Operation
  -> ALLOW / DENY
```

If any required security state cannot be verified, the operation must fail closed.

## Ring And Capability Separation

Rings provide coarse-grained boundaries.

Capabilities provide fine-grained authority.

A capability does not automatically bypass ring policy.

A ring position does not automatically grant capability authority.

Cross-ring operations must be explicit, mediated, and auditable.

## Directory And Security

The Directory Service provides discoverability, not authority.

The following are explicitly false:

```text
Knowing a name grants authority.
Resolving a name grants authority.
Enumerating a directory grants authority.
Owning a name grants authority over the target object.
```

Directory state may influence policy decisions, but directory lookup itself is not access.

## Resource Security

Resource use is authority-bearing.

Resource visibility does not grant control.

Resource allocation must be explicit, owned, accounted, auditable, and reclaimable.

Resource exhaustion must not produce broader authority as a fallback.

Reserved recovery, supervisor, and audit resources must be protected from normal workload exhaustion.

## Audit Security

Audit is evidence.

Audit authority and operational authority are separate.

The ability to read audit data does not imply the ability to control the audited component.

The ability to modify system state does not imply the ability to modify audit history.

Audit suppression, loss, exhaustion, and degradation must be observable.

## Error Security

Unknown state must be explicit.

Unknown state may require degraded operation, quarantine, recovery, or manual review.

Unknown state must not be silently treated as valid.

Authority-bearing operations must fail closed when security state is unknown.

## Supervisor Security

The Supervisor Management Fabric is control-isolated from the Operational Fabric.

The Operational Fabric may observe supervisor state through authorized audit or observation bridges.

The Operational Fabric may not directly control supervisor resources.

Supervisor resources include reset, watchdog, RTC, power, recovery, secure boot state, firmware validation state, and fault logs.

Observation is not control.

## Bootstrap Security

Bootstrap authority is not runtime authority.

Before the runtime security model exists, only narrowly scoped bootstrap authority may exist.

Bootstrap authority may validate firmware, initialize minimal hardware, fetch boot data, record boot audit, start supervisor services, or enter recovery mode.

Bootstrap authority may not become normal runtime administrative authority.

## Compatibility Security

Compatibility environments, including future POSIX or virtual hardware personalities, are objects.

They must have explicit capabilities, bounded resources, lifecycle policy, audit visibility, and cleanup.

Compatibility must not weaken native Atarix security semantics.

## Required Security Events

The following must be auditable:

```text
Capability grant
Capability revoke
Delegation
Authorization denial
Policy override
Ring violation
Resource allocation
Resource exhaustion
Directory mutation
Supervisor action
Bootstrap security failure
Recovery mode entry
Quarantine transition
Cleanup failure
```

## Failure Rule

Atarix prefers denial, quarantine, and explicit degraded operation over assumed safety.

The controlling rule is:

```text
When authority cannot be verified, fail closed.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-003-Capability-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-003-Capability-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-003-Capability-Model.md`

# ATX-SPEC-003 Capability Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines capabilities in Atarix.

A capability is an explicit authority-bearing object or record that permits specific operations against a specific object, service, resource, namespace binding, mailbox endpoint, or controlled interface.

Capabilities are not identities.

Capabilities are not names.

Capabilities are not directory entries.

## Core Rule

```text
Possession of identity is not possession of authority.
```

A caller may know an object exists and still lack a capability to operate on it.

## Capability Properties

A capability must be:

- Explicit
- Scoped
- Auditable
- Revocable where possible
- Bound to intended authority
- Constrained by ring policy
- Constrained by lifecycle state
- Constrained by resource policy
- Constrained by expiration or lease where applicable

## Capability Scope

A capability may be scoped by:

```text
Object identity
Resource identity or pool
Directory binding
Mailbox endpoint
Service interface
Operation set
Ring
Owner
Session
Time
Node
Fabric
Policy domain
```

## Capability Operations

Common operations include:

```text
READ
WRITE
CONTROL
ALLOCATE
RESERVE
OBSERVE
ACCOUNT
DELEGATE
REVOKE
CREATE
DESTROY
EXECUTE
RECOVER
```

Operation names are architectural names. Their wire-format encodings are implementation details.

## Capability And Ring Interaction

Rings provide coarse-grained security boundaries.

Capabilities provide fine-grained authority within or across those boundaries.

A capability does not automatically bypass ring policy.

A ring level does not automatically grant a capability.

Cross-ring use must be explicit and auditable.

## Capability And Directory Interaction

Directory lookup does not return authority.

A directory entry may help locate an object, service, or resource, but the caller must still present or derive a valid capability before performing authority-bearing operations.

## Capability And Resource Interaction

Resource use is authority-bearing.

Capabilities may authorize resource allocation, reservation, observation, accounting, revocation, or control.

Resource visibility does not grant resource control.

## Delegation

Delegation creates a derived authority from an existing authority.

Delegated capabilities must not exceed the authority of the delegator.

Delegation must preserve:

```text
Scope
Operation limits
Ring limits
Resource limits
Lifecycle limits
Expiration
Audit chain
Revocation relationship
```

## Revocation

Revocation invalidates capability authority.

Revocation may require:

```text
Mailbox closure
Resource reclamation
Directory update
Lifecycle transition
Audit event
Quarantine
```

A revoked capability must remain denied after ring checks, resource checks, or directory resolution.

## Expiration And Leases

Capabilities may be time-bounded.

Expiration must not silently convert into persistence.

Expired capabilities must fail closed for authority-bearing operations.

## Capability And Audit

Capability events must be auditable, including:

```text
Capability created
Capability granted
Capability delegated
Capability used
Capability denied
Capability expired
Capability revoked
Capability destroyed
```

Audit records should identify the authority chain used for the decision.

## Capability And Error Handling

If capability validity cannot be verified, the operation must fail closed.

If capability state is unknown after crash or recovery, affected authority should be quarantined or denied until reconciled.

## Compatibility Environments

Compatibility VMs, POSIX personalities, virtual hardware, auxiliary compute providers, and service shims are objects and require explicit capabilities.

Compatibility must not become an ambient authority bridge.

## Required Future Work

- Define concrete capability wire format.
- Define capability identity versus capability record identity.
- Define revocation graph semantics.
- Define capability serialization rules.
- Define compatibility with future Policy Model.
- Define version negotiation for capability records in ATX-SPEC-012.

## Summary

Capabilities are explicit authority.

Knowing, naming, observing, or executing is not enough.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-004-Lifecycle-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-004-Lifecycle-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-004-Lifecycle-Model.md`

# ATX-SPEC-004 Lifecycle Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines lifecycle behavior for Atarix objects, resources, bindings, capabilities, mailboxes, services, and persistent state.

Lifecycle is a security concern.

An object that cannot be cleaned up, revoked, audited, or reconciled becomes permanent attack surface.

## Core Rule

```text
No object is immortal by default.
```

Every object and resource must have an owner, scope, lifetime, cleanup path, persistence policy, audit visibility, and failure behavior.

## Lifecycle Axioms

```text
Persistence is not ownership.
Allocation is not ownership.
Ownership is not authority.
Unknown state must be explicit.
Recovery must not silently regrant authority.
Cleanup failure is a security event.
```

## Suggested Lifecycle States

General object lifecycle states:

```text
CREATED
INITIALIZING
ACTIVE
SUSPENDED
QUIESCING
REVOKING
REVOKED
EXPIRED
DESTROYING
DESTROYED
QUARANTINED
RECOVERING
FAILED
UNKNOWN
```

Not every object supports every state.

State transitions must be explicit and auditable.

## Ownership And Lifetime

Every object has an owner or responsible authority.

The owner is accountable for the object's existence and cleanup.

Ownership does not automatically grant every operation on the object.

Owner disappearance must be handled explicitly by policy.

## Leases

Temporary objects and resources should use leases by default.

A lease must define:

```text
Owner
Start time
Expiration time
Renewal authority
Expiration behavior
Cleanup authority
Audit behavior
```

Lease expiration must not silently become persistence.

## Persistence

Persistent state must be explicitly requested and policy-approved.

Persistent objects must define:

```text
Owner
Reason
Scope
Recovery behavior
Removal path
Audit visibility
Version or schema requirements
```

Persistence is not ownership and does not imply permanent authority.

## Cleanup

Cleanup may include:

```text
Capability revocation
Mailbox closure
Resource release
Directory binding removal
Temporary storage removal
DMA shutdown
Audit record generation
Cache invalidation
Persistent state removal or tombstoning
```

Cleanup failure must be visible to Audit and Error models.

## Quarantine

Quarantine isolates uncertain or unsafe state.

Quarantine may apply to:

```text
Objects
Resources
Directory bindings
Persistent storage
Capabilities
Services
Audit streams
```

Quarantine preserves evidence while preventing unsafe operation.

## Recovery

Recovery transitions an object or resource from failed, unknown, or quarantined state toward known safe state.

Recovery must not silently regrant authority.

Recovery must be auditable.

Recovered state must be distinguishable from uninterrupted normal state.

## Directory Integration

When an object is destroyed, directory bindings must be removed, marked stale, or tombstoned.

Destroyed object identities must not be reused to satisfy stale bindings.

## Resource Integration

When an object exits, expires, is destroyed, or loses authority, its resource allocations must be released, revoked, reconciled, or quarantined.

Resource leaks are architectural defects.

## Audit Integration

The following lifecycle events must be auditable:

```text
Object created
Object activated
Object suspended
Object expired
Object revoked
Object destroyed
Object quarantined
Object recovered
Cleanup started
Cleanup failed
Persistent state created
Persistent state removed
```

## Error Integration

Unknown lifecycle state must be explicit.

If lifecycle state cannot be verified, authority-bearing operations must fail closed.

## Supervisor And Bootstrap Integration

Supervisor objects and bootstrap objects are not exempt from lifecycle rules.

Bootstrap authority must not silently become runtime authority.

Supervisor control state must remain isolated while supervisor audit state remains observable where policy permits.

## Required Future Work

- Define concrete lifecycle state machine.
- Define required transitions for object, resource, capability, mailbox, and binding types.
- Define owner disappearance semantics.
- Define uninstall semantics.
- Define persistent-state cleanup semantics.
- Define recovery and reconciliation in ATX-SPEC-018.

## Summary

Lifecycle makes existence accountable.

If Atarix creates something, it must know who owns it, why it exists, how long it lives, how it is cleaned up, and what happens when cleanup fails.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-005-Mailbox-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-005-Mailbox-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-005-Mailbox-Model.md`

# ATX-SPEC-005 Mailbox Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines Atarix mailbox semantics.

Mailboxes are the primary architectural mechanism for inter-object communication.

Mailbox transport does not grant authority.

A mailbox endpoint allows communication only under the authority, lifecycle, resource, ring, and capability rules applicable to the message and target object.

## Core Rule

```text
Connectivity is not trust.
```

The ability to send a message does not imply the ability to perform the requested operation.

## Mailbox As Object

A mailbox endpoint is an object or object-owned communication resource.

Mailbox endpoints have:

```text
Owner
Lifecycle state
Resource accounting
Authority requirements
Queue limits
Audit policy
Cleanup behavior
Versioned message format
```

## Authority

Mailbox authority may include:

```text
SEND
RECEIVE
OBSERVE
CONFIGURE
CLOSE
DELEGATE
```

Sending a message is separate from authorizing the operation requested by the message.

## Message Validation

A message must be validated for:

```text
Format version
Length
CRC or integrity field where applicable
Message type
Source identity label
Target object identity or binding
Ring policy
Capability authority
Lifecycle state
Resource availability
```

Malformed or unknown messages must fail closed for authority-bearing operations.

## Ring Boundaries

Cross-ring mailbox communication must be explicit and mediated.

A lower-privilege object must not gain authority by sending messages to a higher-privilege mailbox.

A higher-privilege object must not accidentally launder authority through a lower-privilege mailbox.

## Resource Accounting

Mailbox capacity, queue depth, buffer memory, DMA windows, and transport credits are resources.

Mailbox exhaustion must not bypass validation, policy, or audit requirements.

Mailbox resource use must be owned, accounted, bounded, and cleanable.

## Lifecycle

Mailbox endpoints may be:

```text
CREATED
ACTIVE
SUSPENDED
DRAINING
CLOSING
CLOSED
QUARANTINED
FAILED
```

Endpoint closure must clean up pending messages, resource allocations, capabilities, and directory bindings according to policy.

## Audit

Mailbox events that should be auditable include:

```text
Endpoint created
Endpoint closed
Message denied
Malformed message rejected
Cross-ring message accepted
Cross-ring message denied
Queue exhausted
Endpoint quarantined
Transport fault
```

Audit visibility does not imply control over mailbox endpoints.

## Error Handling

Mailbox errors must distinguish:

```text
Malformed message
Unknown message type
Unsupported version
Authorization denied
Target unavailable
Queue exhausted
Timeout
Transport failure
Target quarantined
```

Unknown mailbox state must be explicit.

## Supervisor Mailbox Boundary

Supervisor control interfaces are not ordinary operational mailboxes.

Operational fabric objects may request supervisor actions only through explicitly mediated interfaces.

Supervisor control remains isolated.

Supervisor audit export may be observable through an audit bridge.

Observation is not control.

## Bootstrap Mailboxes

Bootstrap mailboxes, if present, belong to the Bootstrap Security Model.

Bootstrap communication must not be treated as normal runtime authority.

Bootstrap authority is not runtime authority.

## Versioning

Mailbox messages must carry or imply a version.

Unknown versions must fail closed for authority-bearing operations unless explicit compatibility rules permit degraded interpretation.

## Required Future Work

- Define mailbox message wire format.
- Define mailbox endpoint identity.
- Define mailbox resource accounting structures.
- Define version negotiation in ATX-SPEC-012.
- Define policy interaction in ATX-SPEC-013.
- Define supervisor mailbox restrictions in ATX-SPEC-016.

## Summary

Mailboxes provide communication, not trust.

Every authority-bearing request still requires explicit validation.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-006-Object-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-006-Object-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-006-Object-Model.md`

# ATX-SPEC-006 Object Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This specification formalizes the Atarix Object Model.

Objects are the fundamental architectural entities of Atarix.

Services, devices, resources, mailboxes, directory bindings, compatibility environments, virtual hardware, supervisor interfaces, and auxiliary compute providers are represented as objects or object-owned resources.

## Core Rule

```text
Name is not identity.
Identity is not authority.
```

An object may be named, discovered, observed, moved, virtualized, or persisted without changing its identity or authority model.

## Object Identity

An object's identity is:

- Opaque
- Immutable
- Never reused
- Independent of name
- Independent of physical location
- Independent of current provider
- Independent of capability possession

Object identity must not be a memory address, table slot, mailbox number, CPU-local handle, physical bus address, or implementation detail.

## Required Object Metadata

Every object has:

```text
Object identity
Object type
Owner or responsible authority
Lifecycle state
Ring association or security domain
Resource accounting
Audit visibility
Version or schema information
```

## Object Names

Objects may have zero, one, or many names.

Names are namespace bindings.

Names are useful for humans, administration, discovery, and service lookup.

Names are not identity and do not grant authority.

## Object Authority

Possessing an object identity does not grant authority.

Operating on an object requires explicit authority through capability, ownership policy, ring policy, lifecycle state, resource policy, and system policy.

## Object Lifecycle

Objects participate in the Lifecycle Model.

Objects may be active, suspended, revoked, expired, quarantined, recovering, destroyed, failed, or unknown depending on type.

Destroyed object identities must not be reused.

## Object Resources

Objects may own, allocate, reserve, expose, or consume resources.

Resource allocation is authority-bearing and must be explicit.

Object ownership does not automatically imply full authority over all resources associated with the object.

## Object Audit

Object events should be auditable, including:

```text
Object created
Object named
Object rebound
Object activated
Object suspended
Object granted authority
Object denied authority
Object quarantined
Object destroyed
Object recovered
```

Audit visibility does not imply control authority over the object.

## Object Errors

Unknown object state must be explicit.

If object identity, lifecycle state, version, policy, or authority cannot be verified, authority-bearing operations must fail closed.

## Services As Objects

A service is an object that offers one or more interfaces.

Examples:

```text
/service/audit
/service/storage
/service/network
/service/hash
/service/compression
/service/directory
```

Service location is not service identity.

The provider may be a CPU card, auxiliary compute card, storage processor, network card, or future fabric node.

## Virtual Hardware And Compatibility Objects

Virtual hardware, compatibility VMs, POSIX personalities, and legacy environments are objects.

They must have explicit capabilities, bounded resources, lifecycle policy, audit visibility, versioned interfaces, and cleanup.

Compatibility must not become ambient authority.

## Supervisor Objects

Supervisor-facing objects and supervisor audit exports are objects, but supervisor control remains isolated in the Supervisor Management Fabric.

Operational objects may observe supervisor state only through authorized observation or audit bridges.

Observation is not control.

## Local And Remote Objects

An object may be local, remote within the same operational fabric, or eventually remote through a federated fabric.

Rev A requires only one operational fabric.

Physical location must not be exposed as public object identity.

## Required Future Work

- Define object identity wire format.
- Define object type registry.
- Define object schema versioning in ATX-SPEC-012.
- Define service objects in ATX-SPEC-019.
- Define virtual hardware and POSIX compatibility in ATX-SPEC-015.
- Define storage-backed persistent objects in ATX-SPEC-017.

## Summary

Everything visible to Atarix is modeled as an object or object-owned resource.

Objects provide identity and lifecycle context, not automatic authority.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-008-Directory-Service-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-008-Directory-Service-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-008-Directory-Service-Model.md`

# ATX-SPEC-008 Directory Service Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Directory Service Model.

The Directory Service maps human-readable names to object identities. It provides discovery, binding, aliasing, enumeration, lookup, and stale-binding handling for Atarix objects.

The Directory Service does not grant authority.

Lookup is not access.

Knowing a name is not authority.

Resolving an object identity is not authority.

All operations on resolved objects remain subject to ring policy, ownership rules, capability validation, lifecycle state, resource policy, and system policy.

## Design Philosophy

Atarix separates:

```text
Name
Binding
Object identity
Object location
Authority
Capability
Lifecycle state
Resource state
```

The Directory Service answers:

```text
Given this name, what object identity does it currently bind to?
```

It does not answer:

```text
May this caller use the object?
```

That question belongs to the Security, Authority, Capability, Policy, Lifecycle, and Resource models.

## Core Invariants

1. Names are not objects.
2. Names are not capabilities.
3. Object identities are not names.
4. Object identities are not authority.
5. Lookup success does not imply access.
6. Enumeration success does not imply access.
7. Directory visibility does not imply control.
8. Bindings have owners.
9. Bindings have lifecycle state.
10. Bindings have generation metadata.
11. Bindings can become stale.
12. Stale bindings must be observable.
13. Stale bindings must be removable or tombstoned.
14. Destroyed object identities must never be reused to satisfy stale bindings.
15. Directory operations must be auditable.
16. Directory failure must fail closed for authority-bearing operations.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## Terminology

### Name

A human-readable identifier within a namespace.

Example:

```text
/system/directory
/service/audit
/fabric/node/0
/device/network/eth0
/user/donovan/session/0
```

### Binding

A directory record that maps a name or path to an object identity.

### Object Identity

An opaque immutable identity defined by the Object Model.

### Alias

A binding that resolves to another binding or canonical object path.

### Directory Entry

A record containing binding metadata.

### Directory Object

An object that stores and manages directory entries for a namespace subtree.

### Stale Binding

A binding whose target object no longer exists, cannot be verified, has moved without update, or is otherwise invalid.

### Canonical Path

The preferred human-readable path for an object.

An object may have multiple names, but at most one canonical path within a given namespace authority.

## Operational Fabric Plane

The Directory Service operates on the Operational Fabric discovery and service planes.

It is not part of the bulk data plane.

It is not supervisor control authority.

The Operational Fabric may request information from directory services, but directory lookup remains separate from control authority and resource authority.

## Top-Level Namespace Layout

The recommended top-level namespace layout is:

```text
/
  system/
  service/
  device/
  fabric/
  user/
  session/
  temporary/
  audit/
```

## Binding Model

The core model is:

```text
Name
  -> Binding
  -> Object Identity
  -> Object
```

A binding must contain at least:

```text
Binding ID
Name / path component
Target object identity
Binding type
Owner
Creator
Lifecycle state
Generation
Creation time
Last update time
Persistence policy
Lease / expiration policy
Audit policy
Version / schema information
```

## Binding Types

Atarix recognizes the following conceptual binding types:

```text
DIRECT
ALIAS
MOUNT
REDIRECT
TOMBSTONE
STALE
```

Aliases, mounts, and redirects must not create authority or bypass capability checks.

## Lookup Semantics

A lookup operation resolves a name into metadata.

Conceptually:

```text
lookup("/system/directory")
```

returns:

```text
Object identity
Object type hint
Binding ID
Binding generation
Owner
Lifecycle state
Policy reference
Directory authority
Version information
```

Lookup does not return a capability.

Lookup does not grant object access.

Lookup does not imply that future lookup of the same name resolves to the same object unless a binding generation or object identity is pinned.

## Lookup Failure

Lookup may fail because:

```text
Name not found
Malformed path
Permission denied to enumerate or resolve
Directory unavailable
Binding stale
Binding expired
Alias loop detected
Redirect denied
Target object destroyed
Policy unavailable
Unsupported version
Unknown state
```

Authority-bearing operations must fail closed when lookup state cannot be verified.

## Lookup And Authority

The following are explicitly false:

```text
Knowing a path grants authority.
Resolving a path grants authority.
Enumerating a directory grants authority.
Owning a name grants authority over the target object.
Owning an object automatically grants authority over all names bound to it.
```

Authority must be checked separately.

The access path is:

```text
Lookup
  -> Object identity
  -> Ring policy
  -> Ownership / authority policy
  -> Capability validation
  -> Lifecycle validation
  -> Resource policy
  -> Operation
  -> ALLOW / DENY
```

## Directory Visibility

Atarix may allow directory entries to be visible without granting object access.

Visibility itself may still be policy-controlled.

Observation is not control.

## Generation And Version

Binding generation identifies binding evolution.

Version identifies schema or protocol compatibility.

These must not be conflated.

Rebinding must increment generation metadata and create an audit event.

Unsupported binding or reply versions must fail closed for authority-bearing operations unless explicit compatibility rules allow degraded interpretation.

## Lifecycle Integration

Directory entries participate in the Lifecycle Model.

Bindings may be:

```text
CREATED
ACTIVE
SUSPENDED
STALE
EXPIRED
REMOVED
TOMBSTONED
QUARANTINED
UNKNOWN
```

When an object is destroyed:

```text
Object destroyed
  -> capabilities revoked or invalidated
  -> mailboxes closed
  -> directory bindings marked stale, removed, or tombstoned
  -> resources reclaimed
  -> audit record generated
```

No binding may silently resolve to a replacement object after target destruction.

## Temporary And Persistent Bindings

Temporary bindings are lease-oriented by default.

Persistent bindings require explicit policy, recovery behavior, cleanup authority, version handling, and audit visibility.

Persistence must not be accidental.

## Distributed Directory Behavior

The Directory Service must support future distributed operation.

A lookup may eventually resolve to:

```text
Local object
Object on another node in the same fabric
Migrated object
Replicated service
Remote namespace authority
Federated fabric gateway
```

Rev A requires only one operational fabric.

Remote resolution does not imply remote access.

## Service Location Independence

Services are discovered by name, interface, version, and capability requirements, not by physical implementation.

A service may be provided by a CPU card, auxiliary compute card, storage processor, network card, or future fabric node.

Service location is not service identity.

## Bootstrap Considerations

Some early boot services may need name-like resolution before the full Directory Service exists.

That behavior belongs to the Bootstrap Security Model.

Bootstrap lookup must not be treated as normal runtime directory authority.

Bootstrap authority is not runtime authority.

## POSIX Compatibility Considerations

Future POSIX compatibility will map POSIX paths onto Atarix namespace and directory operations.

POSIX path lookup must not import POSIX ambient-authority assumptions into Atarix.

## Audit Requirements

The following operations must be auditable:

```text
Create binding
Remove binding
Rename binding
Rebind name
Create alias
Remove alias
Create mount
Remove mount
Directory lookup denied
Directory lookup failed due to stale target
Directory lookup redirected
Directory cache invalidated
Binding expired
Binding tombstoned
Binding quarantined
Unsupported version rejected
```

## Initial Directory Sprint Scope

Directory Sprint 1 should implement only:

```text
Direct bindings
Human-readable paths
Object identity targets
Lookup
Create binding
Remove binding
Generation counters
Stale binding behavior
Basic tests
```

Directory Sprint 1 should not implement distributed lookup, replication, caching, POSIX path emulation, complex mounts, or policy language.

## Required Tests

Initial tests should verify:

```text
Create binding
Lookup existing binding
Lookup missing binding
Remove binding
Removed binding does not resolve
Rebinding increments generation
Alias loop rejected
Lookup does not grant authority
Stale binding rejected
Destroyed object identity not reused
Malformed path rejected
Temporary binding expiration
Directory mutation requires authority
Unsupported version fails closed
```

## Open Questions

Q-001: What exact path syntax is allowed?

Q-002: What maximum path length should be supported?

Q-003: What maximum component length should be supported?

Q-004: Are paths case-sensitive?

Q-005: Are Unicode names allowed in kernel-visible paths, or only higher-level display names?

Q-006: How are binding IDs represented?

Q-007: What minimum metadata must fit in a wire-format directory reply?

Q-008: Which namespace roots are mandatory at boot?

Q-009: How does the Directory Service recover persistent bindings after crash?

Q-010: What exact audit event format is required?

## Summary

The Directory Service provides discoverability, not authority.

Its central rule is:

```text
Lookup is not access.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-009-Resource-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-009-Resource-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-009-Resource-Model.md`

# ATX-SPEC-009 Resource Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Resource Model.

A resource is anything finite that an object may consume, reserve, control, expose, delegate, or depend upon.

Resource use is authority-bearing and must be explicit.

Resource visibility is not authority.

Resource allocation is not ownership.

Resource cleanup is a security requirement.

## Design Philosophy

Atarix treats resources as first-class architectural entities rather than incidental implementation details.

A resource must be:

- Named or discoverable where appropriate.
- Owned or attributable.
- Accounted.
- Constrained.
- Auditable.
- Revocable where possible.
- Reclaimed or quarantined when no longer valid.

The Resource Model answers:

```text
Who owns this resource?
Who may allocate it?
Who is consuming it?
How long may it live?
What authority keeps it alive?
What happens when it is revoked?
What happens when the owner disappears?
```

## Core Invariants

1. Every resource has an owner or allocator authority.
2. Every resource has a lifecycle state.
3. Every resource has accounting metadata.
4. Every resource has a cleanup path.
5. Temporary resources use leases by default.
6. Persistent resource reservations require explicit policy.
7. Resource visibility does not imply control.
8. Resource allocation does not bypass ring policy.
9. Resource ownership does not automatically grant all operations on the resource.
10. Resource exhaustion must fail safely.
11. Resource cleanup must be auditable.
12. Resource leaks are architectural defects.
13. Resource identities must not expose implementation-private handles as public contracts.
14. Resource state must be inspectable by authorized observers.
15. System resources are not exempt from lifecycle, audit, or cleanup rules.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-007 Namespace Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model
- Future scheduler, network, filesystem, process, and POSIX compatibility specifications

## Resource Classes

Atarix recognizes these conceptual resource classes:

```text
Compute resources
Memory resources
Communication resources
Storage resources
Device resources
Fabric resources
Supervisor resources
Environmental resources
Audit resources
Recovery resources
```

## Compute Resources

Examples:

```text
CPU core
CPU time slice
scheduler queue slot
accelerator context
GPU compute queue
FPGA region
auxiliary compute card service
```

A compute resource may be provided by a CPU card, auxiliary compute card, GPU card, FPGA region, Raspberry Pi-class service card, RISC-V service card, storage processor, or future provider.

CPU capability is not system capability.

A weak primary CPU may still access strong system services through explicit resource and service interfaces.

## Memory Resources

Examples:

```text
CPU-local SRAM
FPGA-local buffers
fabric memory
RAM-card memory
DMA buffer
shared memory region
page frame
kernel heap allocation
audit buffer pool
```

Memory location is not memory authority.

CPU-local SRAM, fabric RAM, RAM-card memory, and supervisor RAM have different authority and failure semantics.

Resources are not pooled merely because they exist.

Resources are pooled only when ownership, authority, failure domains, and recovery semantics are compatible.

## Supervisor Resources

Supervisor resources belong to the Supervisor Management Fabric, not the Operational Fabric.

Examples:

```text
reset line
watchdog
RTC
power rail control
firmware update slot
recovery mode control
health sensor
fault log
supervisor RAM
```

Operational objects may observe supervisor resource state through authorized observation or audit paths, but they must not directly control supervisor resources.

Observation is not control.

Supervisor memory is not operational scratch space.

## Audit Resources

Audit resources include:

```text
Audit ingress queues
High-speed audit buffers
RAM-backed audit journals
Audit persistence queues
Persistent audit storage
Supervisor audit buffers
```

Audit resources should be reserved so normal workload exhaustion does not silently disable audit.

Audit generation should not require synchronous physical storage writes.

RAM-backed audit buffering is a target architecture once fabric RAM cards or compatible memory services are available.

## Resource Identity And Naming

Resources may be named through the Directory Service.

Examples:

```text
/device/network/eth0
/fabric/node/0/cpu/0
/fabric/node/0/memory/local
/fabric/node/0/memory/card0
/system/resource-manager
/system/power/budget
```

Resource names are not resource identities.

Resource identities are not authority.

Resource directory lookup does not grant resource access.

## Resource Ownership

Ownership answers:

```text
Who is responsible for this resource?
```

Ownership does not automatically answer:

```text
What operations are allowed on this resource?
```

Authority is still capability and policy mediated.

## Resource Allocation

A resource allocation must include:

```text
Allocation ID
Resource identity or pool identity
Owner
Requester
Authority used
Scope
Lifecycle state
Creation time
Lease or persistence policy
Quota impact
Cleanup policy
Audit policy
Version or schema information
```

Allocation may be exclusive, shared, read-only, write-capable, control-capable, time-sliced, quota-limited, best-effort, reserved, or borrowed.

## Leases And Quotas

Temporary resource allocations use leases by default.

Quota enforcement must fail safely.

If quota state cannot be verified, new authority-bearing allocations should be denied.

Lease expiration must trigger reclamation, revocation, or quarantine.

## Revocation And Reclamation

Resource revocation may be:

```text
Immediate
Deferred
Graceful
Forced
Quarantined
Policy-denied
Unsupported
```

Resources must be reclaimed when the owner exits, owner is destroyed, lease expires, capability is revoked, session ends, node crashes, fabric partitions, policy changes, or quota enforcement requires it.

Reclamation may destroy, return, quarantine, scrub, or transfer the resource by explicit policy.

## Resource Exhaustion

Resource exhaustion is expected and must be safe.

When resources are exhausted, Atarix must:

- Fail closed for authority-bearing operations.
- Preserve system-critical recovery paths where possible.
- Preserve supervisor functions where possible.
- Preserve audit visibility where possible.
- Avoid granting broader authority as a fallback.
- Audit denied or degraded allocations.
- Prefer controlled degradation over undefined behavior.

## Resource Visibility

Authorized observers may inspect resource state.

Observation may include resource identity, type, owner, allocation state, quota use, lease expiration, lifecycle state, audit state, and health state.

Observation does not imply control.

## Resource And Capability Interaction

Capabilities grant explicit authority over resources.

A resource capability may authorize operations such as:

```text
READ
WRITE
CONTROL
ALLOCATE
RESERVE
DELEGATE
REVOKE
OBSERVE
ACCOUNT
```

## Auxiliary Compute And Buffer Cards

Auxiliary compute cards are resource providers, not primary CPU cards.

Examples:

```text
Raspberry Pi 5-class compute/buffer card
ARM SBC card
RISC-V service card
GPU service card
AI accelerator card
Storage processor card
```

These cards may provide services such as hashing, compression, encryption, audit staging, network offload, netboot caching, storage staging, or ZFS-style scrub assistance.

Their RAM is exposed as explicit fabric resources, not as transparent CPU-local RAM.

## Crash And Recovery

After crash or reboot, the Resource Model must reconcile:

```text
Persistent allocations
Leased allocations
Expired leases
Outstanding reservations
Shared memory state
DMA state
Mailbox state
Directory bindings
Audit state
Owner state
```

Unknown resource state must be quarantined or denied until reconciled.

Recovery must not silently regrant authority.

## Bootstrap Resource Considerations

Some resources exist before the full runtime Resource Manager exists.

Examples:

```text
Boot ROM storage
Supervisor firmware image
Fabric bitstream image
Boot network endpoint
NTP bootstrap time source
Recovery image slot
```

Bootstrap resources belong to the Bootstrap Security Model.

Bootstrap resource authority is not runtime resource authority.

## POSIX Compatibility Considerations

Future POSIX compatibility will map POSIX resources onto Atarix resources.

Examples:

```text
File descriptor -> capability-wrapped resource reference
Process -> application/session object
Socket -> network endpoint resource
Pipe -> mailbox or stream resource
Signal -> lifecycle/control event
```

POSIX resource assumptions must not import ambient authority into Atarix.

## Initial Resource Sprint Scope

Resource Sprint 1 should implement only:

```text
Resource type identifiers
Resource object metadata
Resource ownership metadata
Basic allocation records
Basic release records
Basic lease expiration semantics
Basic accounting counters
Basic tests
```

Resource Sprint 1 should not implement distributed resource management, complex quotas, persistent reservations, cross-fabric federation, POSIX emulation, scheduler integration, storage allocation, or network bandwidth management.

## Required Tests

Initial tests should verify:

```text
Create resource record
Allocate available resource
Deny allocation of unavailable resource
Release resource
Released resource returns to available state
Allocation records owner
Allocation records authority used
Lease expiration reclaims resource
Revoked capability prevents further allocation
Resource lookup does not grant allocation authority
Resource visibility does not grant control authority
Quota denial fails closed
Cleanup removes temporary allocation
Destroyed owner triggers resource reclamation
Malformed resource request is rejected
```

## Open Questions

Q-001: What is the wire format for resource identity?

Q-002: Are resource identities object identities, resource-local identities, or both?

Q-003: What minimum resource metadata must be present in C structures?

Q-004: What are the initial resource classes required for Rev A?

Q-005: Does CPU-local SRAM appear as one resource pool or multiple regions?

Q-006: How are DMA windows represented?

Q-007: How should mailbox capacity be accounted?

Q-008: What is the minimum quota model for early implementation?

Q-009: How should unreclaimable resources be represented?

Q-010: Which resources are reserved for supervisor recovery paths?

Q-011: How does audit survive resource exhaustion?

Q-012: Which resource operations require explicit capability bits?

## Summary

The Resource Model makes resource use explicit, owned, auditable, bounded, and recoverable.

Its central rule is:

```text
Resource use is authority-bearing and must be explicit.
```

Its companion rule is:

```text
Observation is not control.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-010-Audit-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-010-Audit-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-010-Audit-Model.md`

# ATX-SPEC-010 Audit Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Audit Model.

The Audit Model provides accountability, observability, provenance, incident reconstruction, lifecycle visibility, resource accountability, and security-event recording across the entire Atarix architecture.

Audit is a first-class architectural subsystem.

Audit is not debugging.

Audit is not logging.

Audit is evidence.

## Design Philosophy

Atarix assumes:

- Compromise may occur.
- Software may fail.
- Authority may be abused.
- Resources may leak.
- Cleanup may fail.
- Systems may crash.
- Administrators may need to reconstruct events long after they occurred.

The Audit Model exists to answer:

```text
What happened?
Who did it?
What authority was used?
What object was affected?
What resource was consumed?
When did it occur?
Why was it allowed?
What changed?
Can we prove it?
```

## Core Principles

1. Audit is architectural.
2. Audit is append-oriented.
3. Audit records are evidence.
4. Observation is not control.
5. Audit visibility does not imply authority.
6. Audit records must have provenance.
7. Audit records must identify authority used.
8. Audit records must survive routine failures where practical.
9. Audit suppression must be observable.
10. Audit exhaustion must not silently disable auditing.
11. System objects are not exempt.
12. Supervisor events are auditable.
13. Cleanup failures are auditable.
14. Resource exhaustion is auditable.
15. Security failures are auditable.
16. Audit generation should not depend on synchronous physical storage writes.
17. Audit storage state uncertainty must be explicit.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-006 Object Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-011 Error Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model

## Core Rule

Audit authority and operational authority are separate.

The ability to read audit data does not imply the ability to modify system state.

The ability to modify system state does not imply the ability to modify audit history.

Observation is not control.

## Audit Architecture

Conceptually:

```text
Objects
Services
Resources
Directory
Lifecycle
Capabilities
Supervisor Fabric
        ↓
Audit Events
        ↓
Audit Ingress
        ↓
High-Speed Audit Buffer
        ↓
Audit Persistence Queue
        ↓
Audit Storage
        ↓
Audit Queries / Export
```

Audit should be treated as a service architecture rather than a collection of log files.

## Audit Domains

Atarix recognizes multiple audit domains:

```text
Security
Lifecycle
Resource
Directory
Supervisor
Fabric
Administrative
Bootstrap
Storage
Recovery
```

## Audit Record Requirements

Every audit record should contain:

```text
Audit record ID
Timestamp
Timestamp source
Event type
Actor identity label
Authority used
Object identity
Operation
Result
Reason
Record version
```

Additional metadata may include:

```text
Resource ID
Binding ID
Capability ID
Policy ID
Ring
Node
Fabric
Session
Version
Generation
Correlation ID
Supervisor provenance
Storage state
```

## Provenance

Every audit record must identify:

```text
Who generated the record
What authority generated it
What subsystem generated it
Where it was generated
What version generated it
```

Audit records without provenance are lower trust.

## Audit Integrity

Audit records should be:

```text
Append-oriented
Sequence numbered
Tamper evident
Ordered where practical
Correlatable
Versioned
Recoverable where practical
```

A compromised application should not be able to rewrite historical audit records.

## Audit Visibility

Audit data is observable according to policy.

Audit visibility does not imply control.

Audit consumers may include administrators, security services, supervisor services, monitoring systems, diagnostic tools, and restricted applications.

## Audit Buffering And Persistence

Audit generation must not require synchronous physical disk writes.

Audit events should first enter a high-speed append buffer.

Audit persistence should be asynchronous and journaled where practical.

Audit resources shall be reserved and isolated from normal workloads where practical.

Audit exhaustion shall be observable.

Supervisor audit buffering shall be independent of operational audit buffering.

Loss of persistent storage shall degrade audit persistence but must not silently disable audit generation.

## Audit Storage Hierarchy

Target architecture:

```text
L0  Supervisor audit buffer
L1  Operational audit ingress buffer
L2  RAM-backed audit journal
L3  Persistent audit journal
L4  Checksummed copy-on-write audit dataset
L5  Archive / replication target
```

Early Rev A implementations may support only supervisor-local fault logging and small operational ring buffers.

Fabric RAM cards or compatible memory services are expected before serious RAM-backed audit journals become practical.

## ZFS-Inspired Recovery Target

The long-term storage target should support ZFS-like properties:

```text
Checksums
Copy-on-write updates
Snapshots or immutable checkpoints
Replayable journal state
Scrub or verification
Send/receive or export equivalent
```

Direct ZFS use is not required for early Atarix, but the audit persistence model should not preclude ZFS or ZFS-like storage.

## Supervisor Audit Bridge

The Supervisor Management Fabric is control-isolated.

Supervisor events may be exported through a Supervisor Audit Bridge.

Conceptually:

```text
Supervisor Fabric
    ↓
Audit Bridge
    ↓
Operational Audit Service
```

The bridge is observational.

The bridge does not grant operational control over supervisor resources.

Examples of exported events:

```text
Watchdog timeout
Boot validation failure
Recovery entry
Power fault
Firmware update
RTC synchronization
```

## Audit And Resource Exhaustion

Audit must remain observable during resource pressure.

If audit buffers are exhausted:

```text
Audit loss must be recorded.
Audit suppression must be visible.
Audit degradation must be visible.
```

The system must not silently stop auditing.

## Audit And Cleanup

Cleanup operations are auditable.

Cleanup failures are security-relevant events.

Examples:

```text
Lease expiration
Object destruction
Mailbox cleanup
Resource reclamation
Persistent state removal
Directory binding removal
```

## Audit And Security Events

The following should always generate audit records:

```text
Capability grant
Capability revoke
Delegation
Authorization denial
Policy override
Ring violation
Supervisor action
Recovery mode entry
Quarantine transition
Bootstrap security failure
Unsupported version rejection
```

## Audit And Crashes

After crash recovery, audit systems should record:

```text
Crash detected
Recovery started
Recovery completed
Unreconciled state
Quarantined resources
Audit continuity state
```

Unknown state should be visible.

## Audit And Bootstrap

Some audit events occur before the runtime Audit Service exists.

Examples:

```text
Boot ROM validation
Supervisor startup
Fabric initialization
Netboot fetch
Firmware validation
```

These belong to the Bootstrap Security Model.

Bootstrap audit records should be imported into the runtime audit stream where practical.

## Audit And Distributed Systems

Future fabrics may contain multiple audit producers.

Audit records should support node identification, fabric identification, correlation IDs, independent provenance, and version information.

Rev A only requires one operational fabric.

## Audit Retention

Retention is policy controlled.

Deletion, truncation, export, or rollover of audit data should itself be auditable.

## Audit Authority

Audit authority should distinguish:

```text
Observe audit
Query audit
Export audit
Manage retention
Purge audit
Import supervisor audit
Verify audit integrity
```

These are separate authorities.

## Failure Rules

When audit certainty decreases, uncertainty must be visible.

Examples:

```text
Timestamp unavailable
Storage unavailable
Buffer exhausted
Producer unavailable
Supervisor disconnected
Unsupported record version
Integrity check failed
```

The system must not fabricate certainty.

## Initial Audit Sprint Scope

Audit Sprint 1 should implement:

```text
Audit record structure
Event categories
Basic audit service
Append-only event recording
Query by event type
Query by object
Basic tests
```

Audit Sprint 1 should not implement distributed correlation, remote audit federation, cryptographic signing, replication, advanced retention, RAM-drive audit persistence, or cross-fabric audit.

## Required Tests

Initial tests should verify:

```text
Record audit event
Query audit event
Append ordering
Record capability grant
Record capability revoke
Record resource allocation
Record object creation
Record object destruction
Record directory mutation
Record authorization denial
Record cleanup event
Record audit exhaustion event
Record unsupported audit version
Supervisor audit import path
```

## Open Questions

Q-001: What exact audit record wire format should be used?

Q-002: Are audit records objects?

Q-003: Should audit records have immutable object identities?

Q-004: What minimum timestamp precision is required?

Q-005: How should supervisor audit records be imported?

Q-006: What integrity guarantees are required in Rev A?

Q-007: What audit metadata is mandatory?

Q-008: What events are required during bootstrap?

Q-009: How should audit survive storage exhaustion?

Q-010: What audit query language is required?

Q-011: What minimum RAM-backed buffering level requires RAM cards?

## Summary

The Audit Model provides evidence, provenance, accountability, and observability across Atarix.

Its central rule is:

```text
Observation is not control.
```

Its companion rule is:

```text
Audit is evidence.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-011-Error-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-011-Error-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-011-Error-Model.md`

# ATX-SPEC-011 Error Model

## Status

Draft v0.2, reconciled after Architecture Review 001

## Purpose

This document defines the Atarix Error Model.

The Error Model provides a consistent architectural framework for handling failures, uncertainty, degradation, recovery, quarantine, reconciliation, and fault reporting throughout the system.

Errors are architectural events.

Errors are not merely return codes.

Failures are expected.

Unknown state is a first-class condition.

## Design Philosophy

Atarix assumes:

- Hardware fails.
- Software contains defects.
- Resources are exhausted.
- Networks partition.
- Storage becomes inconsistent.
- Audit systems degrade.
- Compromise may occur.
- Recovery is required.

The Error Model exists to answer:

```text
What failed?
How certain are we?
What is the blast radius?
Can operation continue?
Can the state be trusted?
Must the resource be quarantined?
Can recovery occur automatically?
Must authority be revoked?
What evidence exists?
```

## Core Principles

1. Failure is normal.
2. Unknown state is explicit.
3. Fail closed for authority-bearing operations.
4. Preserve evidence.
5. Prefer quarantine over assumption.
6. Degradation must be observable.
7. Recovery must be auditable.
8. Uncertainty must never be hidden.
9. Error handling must not create authority.
10. Recovery must not silently regrant authority.
11. Resource exhaustion is an error condition.
12. Cleanup failure is an error condition.
13. Supervisor failures are auditable.
14. Bootstrap failures are auditable.
15. Observation is not control.
16. Version mismatch is explicit.
17. Storage uncertainty is explicit.
18. Audit uncertainty is explicit.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model

This specification informs:

- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model

## Error Categories

### Validation Errors

```text
Malformed record
Invalid capability
Invalid path
Version mismatch
Schema violation
Unsupported feature
```

### Authority Errors

```text
Access denied
Capability missing
Capability expired
Delegation denied
Ring violation
Policy violation
Authority unknown
```

### Resource Errors

```text
Out of memory
Quota exceeded
Mailbox exhausted
Storage exhausted
DMA unavailable
Allocation denied
Reserved resource unavailable
```

### Lifecycle Errors

```text
Object destroyed
Object expired
Lease expired
Cleanup failure
Invalid state transition
Object quarantined
```

### Directory Errors

```text
Name not found
Binding stale
Alias loop
Redirect denied
Namespace unavailable
Unsupported binding version
```

### Communication Errors

```text
Mailbox timeout
Transport failure
Message corruption
Node unavailable
Partition detected
Unsupported message version
```

### Storage Errors

```text
Read failure
Write failure
Checksum mismatch
Journal replay required
Pool unavailable
Snapshot rollback required
Partial write suspected
```

### Audit Errors

```text
Audit storage unavailable
Audit buffer exhausted
Audit import failure
Audit integrity failure
Audit continuity unknown
Unsupported audit record version
```

### Supervisor Errors

```text
Watchdog event
Firmware validation failure
Recovery entry
Power fault
RTC fault
Supervisor unavailable
Supervisor audit bridge unavailable
```

### Bootstrap Errors

```text
Boot ROM validation failure
Supervisor startup failure
Fabric initialization failure
Netboot failure
Time source unavailable
Recovery image unavailable
```

## Error Severity

Suggested severity levels:

```text
INFO
NOTICE
WARNING
ERROR
CRITICAL
FATAL
UNKNOWN
```

Severity does not determine authority.

Severity helps determine response and audit handling.

## Unknown State

Unknown state is a distinct architectural condition.

Examples:

```text
Storage partially recovered
Node restarted unexpectedly
Resource ownership uncertain
Audit continuity uncertain
Directory reconciliation incomplete
Capability revocation state uncertain
Policy version unsupported
```

Unknown state must not be silently treated as valid.

Unknown state may require quarantine, restricted operation, manual review, or recovery workflow.

## Quarantine

Quarantine isolates potentially unsafe state.

Examples:

```text
Unverified storage
Unreconciled resource
Compromised service
Unknown capability state
Corrupted audit stream
Unsupported-version object
```

Quarantine preserves evidence while preventing unsafe operation.

Quarantine does not imply destruction.

## Fail-Closed Rules

Authority-bearing operations must fail closed when:

```text
Authority cannot be verified
Capability cannot be validated
Policy cannot be evaluated
Directory target cannot be verified
Resource ownership is uncertain
Object lifecycle is unknown
Version semantics are unsupported
Audit requirements cannot be satisfied where mandatory
```

Convenience must not override security.

## Degraded Operation

Some services may continue in degraded mode.

Examples:

```text
Read-only storage
Reduced networking
Limited directory service
Reduced audit retention
Supervisor-only recovery mode
```

Degraded operation must be explicit, auditable, observable, policy controlled, and reversible where practical.

## Recovery States

Suggested recovery states:

```text
NORMAL
DEGRADED
RECOVERING
RECONCILING
QUARANTINED
EMERGENCY
FAILED
UNKNOWN
```

State transitions must be auditable.

## Reconciliation

After crashes, partitions, unsupported versions, or partial recovery, the system may require reconciliation.

Examples:

```text
Resource ownership
Directory bindings
Audit continuity
Lease state
Mailbox state
Capability state
Persistent storage state
```

Reconciliation must not assume correctness.

Recovery must not silently regrant authority.

## Error Reporting Requirements

Every error should include:

```text
Error category
Error code
Subsystem
Object identity if applicable
Resource identity if applicable
Authority context
Timestamp
Severity
Recovery recommendation
Version context if applicable
Audit correlation ID if available
```

## Audit Integration

All significant errors must generate audit events.

Particularly:

```text
Authority failures
Recovery actions
Quarantine actions
Resource exhaustion
Supervisor failures
Audit failures
Bootstrap failures
Unsupported version rejections
Storage integrity failures
```

## Supervisor Integration

Supervisor failures should be exported through the Supervisor Audit Bridge.

Supervisor control isolation remains in effect.

Observation is not control.

## Resource Exhaustion

Resource exhaustion is expected.

The system should preserve recovery paths, supervisor functions, and audit visibility where possible.

Resource exhaustion must not create authority bypasses.

## Bootstrap Errors

Bootstrap failures belong to the Bootstrap Security Model.

Bootstrap errors should be imported into runtime audit where practical.

Bootstrap error handling must not grant runtime authority.

## Storage And Persistence Errors

Storage errors may require replay, rollback, quarantine, scrub, import refusal, or manual intervention.

Unknown storage state must remain visible.

Partially committed audit or object state must be replayed, discarded, or quarantined deterministically by future Storage and Recovery specifications.

## Crash Recovery

After a crash:

```text
Detect
Audit
Quarantine uncertainty
Reconcile state
Restore services
Record recovery result
```

Unknown state must remain visible until resolved.

## Initial Error Sprint Scope

Error Sprint 1 should implement:

```text
Error categories
Error codes
Severity model
Unknown state support
Quarantine support
Basic tests
```

Error Sprint 1 should not implement full recovery workflows, storage replay, distributed reconciliation, or policy language.

## Required Tests

Initial tests should verify:

```text
Validation error
Authority error
Resource exhaustion
Directory failure
Quarantine transition
Recovery transition
Unknown state handling
Audit generation for errors
Unsupported version failure
Fail-closed authority check
```

## Open Questions

Q-001: What exact error code namespace should be used?

Q-002: Are error records objects?

Q-003: How are errors correlated across services?

Q-004: What error severity is required for supervisor action?

Q-005: How does degraded operation interact with policy?

Q-006: What is the minimum quarantine API?

Q-007: How are bootstrap errors imported into runtime audit?

Q-008: What storage errors require immediate quarantine?

## Summary

The Error Model treats failure, uncertainty, recovery, and quarantine as first-class architectural concepts.

Its central rule is:

```text
Unknown state must be explicit.
```

Its companion rule is:

```text
When authority cannot be verified, fail closed.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-014-Bootstrap-Security-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-014-Bootstrap-Security-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-014-Bootstrap-Security-Model.md`

# ATX-SPEC-014 Bootstrap Security Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Bootstrap Security Model.

The Bootstrap Security Model describes how Atarix establishes trust from reset, validates early firmware and boot artifacts, constrains pre-runtime authority, records bootstrap evidence, enters recovery, and hands control to the runtime security architecture.

Bootstrap exists to establish runtime trust.

Bootstrap is not runtime.

## Core Rule

```text
Bootstrap authority is not runtime authority.
```

Bootstrap authority exists only to validate, initialize, measure, discover, load, recover, and transfer control.

It must not become permanent administrative authority.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model

This specification informs:

- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## Power-On Assumptions

At power-on there is no runtime:

```text
Directory Service
Capability system
Runtime Policy engine
Runtime Audit service
Runtime Service registry
User authority
General-purpose operating system
```

Therefore early trust must come from a minimal root of trust, immutable or protected boot code, hardware identity, signed boot manifests, measured artifacts, and recovery policy.

## Bootstrap Authority

Bootstrap authority may:

```text
Initialize minimal hardware
Read hardware profile information
Validate firmware
Validate FPGA bitstreams
Validate boot manifests
Acquire advisory time
Acquire boot images
Record bootstrap audit events
Start supervisor services
Enter recovery mode
Transfer control to runtime
```

Bootstrap authority may not:

```text
Grant arbitrary runtime capabilities
Create permanent runtime administrator authority
Modify runtime policy without recovery authorization
Access user data as a convenience fallback
Become a general remote administration shell
Silently persist after runtime handoff
```

## Chain Of Trust

Conceptual chain:

```text
Root of Trust / ROM
    -> Supervisor Boot Firmware
    -> Fabric Firmware / Bitstream
    -> Bootstrap Loader
    -> Boot Manifest
    -> Kernel Image
    -> Runtime Supervisor
    -> Runtime Services
```

Each stage must validate or measure the next stage before transferring control.

If validation cannot be completed, the system must enter degraded, recovery, quarantine, failed, or unknown boot state rather than continuing silently.

## Hardware-Bound Boot Artifacts

Boot artifacts must be bound to an authorized hardware profile and signed provenance identity.

A boot artifact signed for one hardware profile must not boot on a different hardware profile unless explicit compatibility policy permits it.

Hardware binding should support layers such as:

```text
Hardware family
Hardware revision
CPU card type
Fabric board type
Supervisor hardware type
Security profile
Optional device identity
```

Binding only to an exact board serial should be optional, because per-machine images are not always desirable.

## Signed Source And Build Provenance

Boot artifacts should include signed source and build provenance.

A boot manifest should contain at least:

```text
Boot manifest version
Hardware profile ID
Hardware profile version
Hardware revision
CPU card type
Fabric board type
Supervisor firmware target
FPGA bitstream target
Boot image hash
Source commit hash
Source reference signature
Build manifest hash
Build toolchain identity
Signature key ID
Signature
```

A boot artifact without verifiable source or build provenance is untrusted unless explicit recovery policy permits a restricted recovery boot.

## Boot Verification Flow

Conceptual flow:

```text
Power on
  -> Read hardware identity / profile
  -> Load boot manifest
  -> Verify manifest version
  -> Verify manifest signature
  -> Verify artifact hashes
  -> Verify hardware target compatibility
  -> Verify source and build provenance
  -> Record bootstrap audit
  -> Boot, degrade, recover, quarantine, or fail
```

## Signature Failure Rules

The following are bootstrap security failures:

```text
Manifest signature failure
Boot artifact hash mismatch
Hardware profile mismatch
Source provenance mismatch
Build manifest mismatch
Unsupported boot manifest version
Unsupported hardware profile version
Unknown signing key
Revoked signing key
```

Authority-bearing boot must fail closed when these failures cannot be resolved by explicit recovery policy.

## Measurements

Bootstrap should measure:

```text
Root firmware hash
Supervisor firmware hash
Fabric bitstream hash
Bootstrap loader hash
Boot manifest hash
Kernel image hash
Configuration hash
Recovery image hash
```

Measurements should be recorded in bootstrap audit buffers and imported into runtime audit where practical.

## Early Audit

Before runtime audit exists, bootstrap audit uses early buffers.

Conceptual path:

```text
ROM / early boot buffer
  -> Supervisor audit buffer
  -> Runtime audit import
```

Bootstrap audit must preserve provenance.

Bootstrap audit import must not create runtime authority.

## Early Time

Time before runtime is uncertain.

Preferred order:

```text
RTC
  -> Supervisor time
  -> Network time
  -> Runtime time service
```

Bootstrap NTP or SNTP is advisory unless validated by policy.

Authority decisions must not depend solely on untrusted early network time.

Time uncertainty must be visible in audit records.

## Pre-OS Network Services

Bootstrap networking is narrowly scoped.

Allowed functions:

```text
Link setup
Static IP or DHCP
ARP
UDP
TFTP
HTTP or HTTPS boot fetch
NTP or SNTP
Boot log upload where policy permits
```

Forbidden functions:

```text
General remote shell
General socket API
User applications
Unmediated remote administration
Runtime network authority
```

Pre-OS networking exists only to bring the system up or recover it.

It is not the runtime network stack.

## Supervisor Startup

The Supervisor Management Fabric should be available before normal runtime services.

Conceptual order:

```text
Power on
  -> Supervisor startup
  -> Fabric initialization
  -> Bootstrap services
  -> Kernel verification
  -> Runtime handoff
```

Supervisor failure must be auditable where possible and must not silently fall back to insecure runtime boot.

## Recovery Mode

Recovery mode is a bootstrap function.

Recovery must be available when normal runtime cannot start.

Examples:

```text
Kernel missing
Boot image invalid
Storage corrupted
Network unavailable
Directory unavailable
Policy database corrupted
Supervisor detects unsafe state
```

Recovery mode is restricted.

Recovery mode must not become permanent administrative runtime.

## Recovery Netboot Image Generation

Recovery mode may produce a signed, hardware-bound netboot image sufficient to restore normal boot.

Conceptual flow:

```text
Boot fails
  -> Enter recovery mode
  -> Validate hardware profile
  -> Fetch or build boot source
  -> Create boot manifest
  -> Bind manifest to hardware profile
  -> Include source and build provenance
  -> Sign manifest and image
  -> Netboot signed recovery-produced image
  -> Import recovery audit trail into runtime audit
```

Recovery signing keys are not normal runtime keys.

Recovery-built images must be hardware-profile-bound.

Recovery-built images must include source and build provenance where practical.

Recovery build and signing actions must be audited.

Recovery-produced boot images may grant only the minimum authority required to boot and reconcile state.

Recovery mode must not become a general-purpose build farm, remote shell, or unrestricted administrator environment.

## Recovery Signing Authority

Recovery signing authority is a distinct authority class.

It may sign:

```text
Recovery netboot manifests
Recovery boot images
Recovery metadata
```

It may not sign arbitrary runtime authority grants unless explicit recovery policy permits and audit records the action.

Recovery signing keys should be protected by supervisor or hardware-backed policy where practical.

## Bootstrap States

Suggested bootstrap states:

```text
BOOT_OK
BOOT_DEGRADED
BOOT_RECOVERY
BOOT_QUARANTINED
BOOT_FAILED
BOOT_UNKNOWN
```

Unknown boot state must be explicit.

## Measured Degradation

Not every bootstrap failure has the same consequence.

Example:

```text
Firmware valid, network unavailable
  -> BOOT_DEGRADED may be permitted.

Firmware validation unavailable
  -> BOOT_RECOVERY or BOOT_QUARANTINED is required.
```

Degraded boot must be policy-controlled and auditable.

## Bootstrap Cleanup And Handoff

When runtime starts, bootstrap authority expires.

Runtime handoff should include:

```text
Boot state
Hardware profile
Verified manifest identity
Image hashes
Measurement records
Time confidence
Network boot source
Recovery status
Bootstrap audit buffer
```

After handoff, bootstrap interfaces must be disabled, restricted, or placed under supervisor control.

## Versioning Requirements

Bootstrap records must be versioned.

Versioned items include:

```text
Boot manifest
Hardware profile
Measurement record
Recovery manifest
Firmware metadata
Netboot protocol profile
```

Unknown versions fail closed for authority-bearing boot decisions unless explicit recovery policy permits restricted recovery.

## Policy Requirements

Bootstrap policy determines:

```text
Allowed hardware profiles
Allowed signing keys
Allowed source references
Allowed recovery images
Allowed degraded boot states
Allowed network boot sources
Allowed recovery signing behavior
```

If bootstrap policy cannot be evaluated, authority-bearing boot must fail closed or enter recovery.

## Audit Requirements

Bootstrap must audit:

```text
Power-on event
Hardware profile detected
Manifest verification result
Signature verification result
Hash verification result
Source provenance result
Build provenance result
Boot decision
Degraded boot decision
Recovery entry
Recovery image generation
Recovery signing action
Runtime handoff
```

## Error Requirements

The Error Model must support bootstrap errors such as:

```text
BOOT_MANIFEST_INVALID
BOOT_SIGNATURE_INVALID
BOOT_HARDWARE_PROFILE_MISMATCH
BOOT_SOURCE_PROVENANCE_INVALID
BOOT_BUILD_PROVENANCE_INVALID
BOOT_POLICY_UNAVAILABLE
BOOT_RECOVERY_REQUIRED
BOOT_UNKNOWN_STATE
RECOVERY_SIGNING_DENIED
```

## Initial Bootstrap Sprint Scope

Bootstrap Sprint 1 should define:

```text
Boot manifest structure
Hardware profile identifiers
Signature verification result codes
Boot state enum
Bootstrap audit event types
Recovery netboot manifest concept
Basic tests
```

Bootstrap Sprint 1 should not implement:

```text
Full secure boot hardware integration
Remote attestation
Full recovery build system
Policy language
Distributed boot federation
General remote administration
```

## Required Tests

Initial tests should verify:

```text
Valid manifest accepted
Invalid signature rejected
Hardware profile mismatch rejected
Unsupported manifest version rejected
Source provenance mismatch rejected
Recovery mode entered after boot failure
Recovery-produced manifest includes hardware profile
Recovery-produced image requires recovery signing authority
Bootstrap authority expires at runtime handoff
Bootstrap audit records generated
Unknown boot state fails closed
```

## Required Future Work

- Define boot manifest wire format.
- Define hardware profile schema.
- Define signing key lifecycle.
- Define recovery signing key storage.
- Define bootstrap audit import format.
- Define supervisor integration in ATX-SPEC-016.
- Define persistent recovery storage in ATX-SPEC-017.
- Define reconciliation after recovery in ATX-SPEC-018.

## Summary

The Bootstrap Security Model establishes trust from reset without turning bootstrap authority into runtime authority.

Its central rules are:

```text
Bootstrap authority is not runtime authority.
Boot artifacts must be hardware-profile-bound and provenance-verifiable.
Recovery may generate signed netboot images, but only under constrained recovery authority.
When bootstrap trust cannot be verified, fail closed or enter recovery.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-016-Supervisor-Management-Fabric.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-016-Supervisor-Management-Fabric.md -->
### Integrated source: `docs/architecture/ATX-SPEC-016-Supervisor-Management-Fabric.md`

# ATX-SPEC-016 Supervisor Management Fabric

## Status

Draft v0.1

## Purpose

This document defines the Atarix Supervisor Management Fabric.

The Supervisor Management Fabric is the isolated management and recovery fabric responsible for reset, power, watchdogs, RTC, health monitoring, fault logging, recovery coordination, secure bootstrap support, firmware validation, and supervisor audit export.

The Supervisor Management Fabric is not the Operational Fabric.

Supervisor authority is isolated.

Supervisor observability is shared only through controlled audit or observation paths.

## Core Rule

```text
The Supervisor Fabric is control-isolated but audit-visible.
```

The Operational Fabric may request supervisor actions.

The Operational Fabric may observe supervisor events where policy permits.

The Operational Fabric may not directly control supervisor authority.

Observation is not control.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model

This specification informs:

- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-018 Recovery and Reconciliation Model
- ATX-SPEC-019 Service Model

## Design Philosophy

Traditional systems often make the management plane either too powerful and reachable, or too opaque and unauditable.

Atarix separates:

```text
Supervisor control authority
Supervisor observation authority
Operational runtime authority
Recovery authority
Audit export authority
```

The Supervisor Management Fabric exists to preserve recovery and trustworthy observation even when the Operational Fabric is compromised, crashed, wedged, or not yet running.

## Fabric Separation

Atarix recognizes at least two fabric classes:

```text
Operational Fabric
Supervisor Management Fabric
```

### Operational Fabric

The Operational Fabric provides runtime discovery, control, service, mailbox, DMA, memory, storage, networking, and data-plane operations.

### Supervisor Management Fabric

The Supervisor Management Fabric provides management, recovery, and health functions.

It should be physically or logically isolated from normal operational traffic.

The exact Rev A implementation may use RP2350-class supervisor hardware, sideband links, GPIOs, serial links, management mailboxes, or other constrained channels.

## Supervisor Authority

Supervisor authority may include:

```text
Assert reset
Release reset
Power rail control
Watchdog arm / disarm / service
RTC read / constrained write
Enter recovery mode
Validate firmware state
Validate boot state
Record fault state
Export supervisor audit
Coordinate safe shutdown
Coordinate emergency isolation
```

Supervisor authority must not be granted to arbitrary runtime objects.

## Operational Requests

Operational objects may request supervisor actions only through mediated request paths.

Conceptually:

```text
Operational Object
  -> Supervisor Request Gateway
  -> Policy Check
  -> Supervisor Command Evaluator
  -> Supervisor Action or Denial
```

A request is not a command.

A valid operational capability is not supervisor authority.

Supervisor remains the final authority for supervisor-controlled actions.

## Forbidden Direct Control

The Operational Fabric must not directly control:

```text
Reset lines
Power rails
Watchdog disable
Firmware validation state
Recovery mode override
Supervisor signing keys
Supervisor audit buffer mutation
Secure boot decision state
```

Any operational path that can modify these functions must be treated as a supervisor request path and policy-controlled.

## Supervisor Resources

Supervisor resources include:

```text
Reset lines
Power rails
Watchdog timers
RTC
Health sensors
Fault logs
Recovery controls
Firmware validation state
Boot validation state
Recovery signing state
Supervisor audit buffers
Supervisor-local storage
```

Supervisor resources are not ordinary operational resources.

Supervisor RAM and supervisor storage are not operational scratch space.

## Supervisor Audit Bridge

Supervisor events may be exported into the Operational Audit Service through a Supervisor Audit Bridge.

Conceptually:

```text
Supervisor Fabric
  -> Supervisor Audit Buffer
  -> Audit Bridge
  -> Operational Audit Service
```

The Audit Bridge is observational.

It must not allow operational control over supervisor state.

## Supervisor Audit Events

Supervisor audit should include:

```text
Power-on event
Reset asserted
Reset released
Watchdog armed
Watchdog fired
Watchdog serviced
Recovery mode entered
Firmware validation passed
Firmware validation failed
Boot manifest accepted
Boot manifest rejected
RTC adjusted
Power rail fault
Health threshold exceeded
Supervisor request received
Supervisor request denied
Supervisor request executed
Supervisor audit export event
```

Audit events must preserve provenance.

## Command Model

Supervisor commands must be minimal and explicit.

Each command should define:

```text
Command ID
Command version
Required authority
Required policy
Allowed caller class
Expected state
Result code
Audit requirement
Failure behavior
```

Supervisor commands must not be open-ended scripting interfaces.

## Request Results

Supervisor request results may include:

```text
EXECUTED
DENIED
DEFERRED
REQUIRES_APPROVAL
QUARANTINED
UNSUPPORTED
UNKNOWN
FAILED
```

Authority-bearing uncertainty must fail closed.

## Supervisor Lifecycle

Supervisor components participate in lifecycle management.

Suggested states:

```text
RESET
INITIALIZING
ACTIVE
DEGRADED
RECOVERY
QUARANTINED
FAILED
UNKNOWN
```

Supervisor state transitions must be auditable where possible.

## Watchdog Model

Watchdogs are supervisor resources.

Watchdog disable or reconfiguration requires supervisor authority and policy approval.

Operational services may report liveness, but they may not unilaterally disable watchdog protection.

## Reset And Power Model

Reset and power controls are supervisor authority.

Operational runtime may request reset, power cycle, or shutdown.

Supervisor policy decides whether to execute the request.

Unsafe power or reset operations should require audit and may require approval depending on policy.

## RTC And Time Model

RTC is a supervisor resource.

Runtime may observe supervisor time or request time adjustment.

Time modification requires policy.

Bootstrap time uncertainty must be visible in audit records.

## Firmware And Boot Validation

Supervisor participates in firmware and boot validation as defined by the Bootstrap Security Model.

Supervisor may track:

```text
Firmware version
Firmware hash
FPGA bitstream hash
Boot manifest identity
Hardware profile
Recovery signing state
```

Supervisor boot validation state must not be modifiable by normal operational services.

## Recovery Coordination

Supervisor may enter or coordinate recovery when:

```text
Boot validation fails
Watchdog fires
Operational fabric is wedged
Runtime audit is unavailable
Storage state is unsafe
Policy database is unavailable
Hardware fault is detected
```

Recovery coordination must not silently grant runtime authority.

## Recovery Signing Interaction

Recovery signing authority is distinct from runtime authority.

Supervisor may protect recovery signing state or mediate recovery signing requests.

Recovery signing actions must be audited.

Recovery signing keys must not be exposed as operational resources.

## Off-Board Or Remote Management

Off-board supervisor access, if supported, must use stronger protection than on-board sideband wiring.

Requirements may include:

```text
Cryptographic authentication
Integrity protection
Replay protection
Rate limiting
Explicit enablement
Audit logging
Physical presence policy where applicable
```

No unauthenticated remote supervisor control is permitted.

## Encryption And Authentication

Encryption alone is not isolation.

On-board paths may rely on physical/logical isolation plus authenticated commands.

Off-board or remote paths require cryptographic authentication and integrity protection.

Confidentiality should be added where supervisor data exposure is sensitive.

## Failure Behavior

Supervisor failure must be explicit.

Supervisor failure states include:

```text
Supervisor unavailable
Supervisor degraded
Supervisor command path unavailable
Supervisor audit unavailable
Supervisor policy unavailable
Supervisor recovery unavailable
```

If supervisor state cannot be trusted, runtime authority-bearing operations depending on supervisor state must fail closed or enter recovery.

## Policy Requirements

Supervisor policy determines:

```text
Allowed supervisor commands
Allowed requesters
Approval requirements
Recovery triggers
Watchdog behavior
Power behavior
RTC adjustment rules
Remote access rules
Audit export rules
```

Policy failure must not create supervisor authority.

## Versioning Requirements

Supervisor protocols, commands, audit exports, firmware metadata, and recovery metadata must be versioned.

Unknown supervisor protocol versions fail closed for authority-bearing operations unless explicit recovery policy permits restricted recovery behavior.

## Security Requirements

The Supervisor Management Fabric must satisfy:

```text
Control isolation from Operational Fabric
Minimal command surface
Explicit request mediation
Policy-controlled execution
Audit-visible decisions
No ambient supervisor authority
No operational access to supervisor keys
No operational scratch use of supervisor memory
Fail-closed uncertainty
```

## Initial Supervisor Sprint Scope

Supervisor Sprint 1 should define:

```text
Supervisor state enum
Supervisor request result enum
Supervisor command identifiers
Supervisor audit event identifiers
Supervisor request gateway concept
Watchdog event model
Reset request model
Basic tests
```

Supervisor Sprint 1 should not implement:

```text
Full remote management
Cryptographic remote console
Complex policy language
Distributed supervisor federation
General supervisor scripting
```

## Required Tests

Initial tests should verify:

```text
Operational request is not direct command
Unauthorized supervisor command denied
Supervisor request audit generated
Watchdog event audit generated
Reset request denied without authority
Recovery request enters recovery state
Supervisor audit export is observational only
Operational fabric cannot mutate supervisor audit buffer
Unknown supervisor protocol version fails closed
Supervisor authority does not become runtime authority
```

## Required Future Work

- Define supervisor command wire format.
- Define supervisor audit export format.
- Define supervisor request gateway C API.
- Define RP2350 Rev A supervisor mapping.
- Define supervisor key storage requirements.
- Define remote management policy.
- Define integration with Storage and Persistence Model.
- Define integration with Recovery and Reconciliation Model.

## Summary

The Supervisor Management Fabric is the control-isolated management and recovery fabric for Atarix.

Its central rules are:

```text
The Supervisor Fabric is control-isolated but audit-visible.
Observation is not control.
A request is not a command.
Supervisor authority is not runtime authority.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/ATX-SPEC-018-Recovery-and-Reconciliation-Model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/ATX-SPEC-018-Recovery-and-Reconciliation-Model.md -->
### Integrated source: `docs/architecture/ATX-SPEC-018-Recovery-and-Reconciliation-Model.md`

# ATX-SPEC-018 Recovery and Reconciliation Model

## Status

Draft v0.1

## Purpose

This document defines the Atarix Recovery and Reconciliation Model.

Recovery describes how Atarix returns from failure, degradation, crash, corruption, version mismatch, audit discontinuity, or unsafe state to a known and policy-approved operating state.

Reconciliation describes how Atarix proves that system subsystems agree with each other after failure or recovery.

Recovery is not administrative access.

Recovery is not authority restoration.

## Core Rule

```text
Recovery restores trust.
Recovery does not restore authority.
```

Recovery may verify, repair, reconcile, rebuild, quarantine, replay, or report state.

Recovery may not bypass policy, create ownership, grant arbitrary capabilities, suppress audit, or silently restore authority.

## Relationship To Other Specifications

This specification depends on:

- ATX-SPEC-001 Security Model
- ATX-SPEC-002 Authority Model
- ATX-SPEC-003 Capability Model
- ATX-SPEC-004 Lifecycle Model
- ATX-SPEC-008 Directory Service Model
- ATX-SPEC-009 Resource Model
- ATX-SPEC-010 Audit Model
- ATX-SPEC-011 Error Model
- ATX-SPEC-012 Versioning Model
- ATX-SPEC-013 Policy Model
- ATX-SPEC-014 Bootstrap Security Model
- ATX-SPEC-016 Supervisor Management Fabric
- ATX-SPEC-017 Storage and Persistence Model
- ATX-SPEC-019 Service Model

This specification informs:

- ATX-SPEC-015 POSIX Compatibility Model
- ATX-SPEC-021 Memory and Data Movement Model

## Design Philosophy

Failure is expected.

Recovery must prove safety rather than assume it.

The system may restart, reload, replay, rebuild, or reattach components, but none of those actions automatically restore authority.

The system is not fully recovered until required reconciliation has completed.

## Recovery Domains

Recovery may apply to:

```text
Bootstrap state
Supervisor state
Directory state
Storage state
Service state
Resource state
Capability state
Audit state
Policy state
Compatibility environments
Memory and buffer state
```

Each domain may require different recovery procedures, but all recovery domains must preserve auditability and fail closed when authority is uncertain.

## Recovery Authority

Recovery authority is a distinct authority class.

Recovery authority may:

```text
Inspect state
Verify integrity
Replay journals
Import audit records
Rebuild indexes
Reconcile metadata
Enter quarantine
Produce recovery reports
Request supervisor-mediated recovery
```

Recovery authority may not:

```text
Create administrator authority
Grant arbitrary capabilities
Bypass policy
Suppress audit
Silently override ownership
Silently restore revoked authority
```

Recovery authority is temporary and scoped.

## Recovery States

Suggested recovery states:

```text
RECOVERY_PENDING
RECOVERY_ACTIVE
RECOVERY_DEGRADED
RECOVERY_RECONCILING
RECOVERY_QUARANTINED
RECOVERY_COMPLETE
RECOVERY_FAILED
RECOVERY_UNKNOWN
```

Unknown recovery state must be explicit.

## Recovery Objects

Recovery artifacts are objects.

Examples:

```text
Recovery Manifest
Recovery Journal
Recovery Snapshot
Recovery Report
Recovery Audit Package
Recovery Image
Recovery Policy
Recovery Signing Record
```

Recovery objects are versioned, lifecycle-managed, auditable, and policy-controlled.

## Reconciliation

Recovery and reconciliation are distinct.

Recovery asks:

```text
Can the system operate?
```

Reconciliation asks:

```text
Does the system agree with itself?
```

A system may be booted but not reconciled.

A system may be partially operational but still degraded, quarantined, or pending reconciliation.

## Reconciliation Targets

Reconciliation may compare:

```text
Directory vs Object Store
Directory vs Service Registry
Storage vs Audit
Capabilities vs Objects
Capabilities vs Revocation State
Resources vs Allocations
Services vs Directory Bindings
Supervisor vs Runtime State
Bootstrap Audit vs Runtime Audit
Policy Records vs Runtime Policy Cache
```

Mismatch must be explicit.

## Reconciliation Outcomes

Reconciliation may produce:

```text
CONSISTENT
REPAIRED
REBUILT
QUARANTINED
MANUAL_REVIEW_REQUIRED
FAILED
UNKNOWN
```

Unknown reconciliation state must fail closed for authority-bearing operations that depend on the unreconciled state.

## Journals

Recovery may use journals.

Examples:

```text
Directory Journal
Storage Journal
Audit Journal
Service Registration Journal
Resource Allocation Journal
Policy Journal
```

Journal replay must be:

```text
Version-aware
Generation-aware
Deterministic where practical
Auditable
Policy-controlled
```

Journal replay must not silently restore authority.

## Snapshots

Recovery snapshots are objects.

Recovery snapshots should be:

```text
Read-only by default
Versioned
Auditable
Integrity-checked
Policy-controlled
```

A snapshot preserves state.

A snapshot is not authority.

## Quarantine

Quarantine isolates unsafe or uncertain state while preserving evidence.

Quarantine is required when:

```text
Ownership is uncertain
Authority is uncertain
Integrity is uncertain
Version is unsupported
Policy cannot be evaluated
Audit continuity is broken
Provider behavior is unsafe
Storage provenance is uncertain
```

Core rule:

```text
When trust cannot be established, quarantine beats execution.
```

## Recovery And Audit

Every recovery action should generate audit.

Minimum fields:

```text
Recovery ID
Actor identity label
Recovery authority used
Object or resource identity
Operation
Result
Reason
Timestamp
Version
Generation where applicable
```

Audit continuity must be reconciled before recovered state is considered fully trusted.

## Recovery And Bootstrap

Recovery integrates with the Bootstrap Security Model.

Conceptual flow:

```text
Boot failure
  -> Recovery mode
  -> Signed recovery netboot image where needed
  -> Runtime startup
  -> Runtime recovery import
  -> Reconciliation
  -> Normal, degraded, or quarantined operation
```

A system booted from a recovery-generated image is not fully recovered until reconciliation completes.

## Recovery And Supervisor

Supervisor may trigger or coordinate recovery.

Examples:

```text
Watchdog fired
Power failure detected
Firmware validation failure
Storage fault detected
Runtime failed to report liveness
Operational fabric wedged
```

Supervisor-triggered recovery must be auditable.

Supervisor triggering recovery does not automatically determine the recovery outcome.

## Recovery And Storage

Storage recovery may include:

```text
Checksum verification
Scrubbing
Journal replay
Snapshot recovery
Replication repair
Object reconciliation
Quarantine of unsafe objects
```

Corrupted storage must never become authority.

## Recovery And Services

A service restart is not reconciliation.

Before a recovered service becomes active, recovery must verify:

```text
Service identity
Service provider identity
Service version
Service lifecycle state
Service policy
Service authority
Service resource ownership
Audit continuity where applicable
```

## Recovery And Directory

Directory recovery may include:

```text
Binding validation
Generation verification
Namespace rebuild
Alias validation
Tombstone verification
Object target verification
```

Directory corruption must not create authority.

Lookup remains not access.

## Recovery And Capabilities

Capabilities must be revalidated after recovery when their backing state may have changed.

Recovery must verify:

```text
Capability exists
Capability version is supported
Capability target still exists
Capability is not revoked
Capability has not expired
Capability policy still permits use
```

Recovery must not silently re-enable revoked capabilities.

## Recovery And Resources

Resource recovery may include:

```text
Allocation reconstruction
Lease expiration
Quota reconciliation
Orphan detection
Resource quarantine
Resource release
```

Resource leaks discovered during recovery are audit-relevant events.

## Recovery Cleanup Escalation

Recovery cleanup builds on the Policy Cleanup Escalation Model.

Suggested recovery cleanup levels:

```text
L1 Recovery Cleanup
L2 Recovery Quarantine
L3 Recovery Snapshot
L4 Recovery Isolation
L5 Supervisor Recovery
L6 Manual Recovery
```

Escalation moves toward:

```text
More audit
Less authority
More containment
Greater evidence preservation
```

Escalation must not create broader authority.

## Replay

Replay may be used for:

```text
Audit import
Directory reconstruction
Storage reconstruction
Service registration reconstruction
Resource allocation reconstruction
```

Replay must be version-aware, generation-aware, auditable, and deterministic where practical.

Replay must not assume compatibility when versioning is unknown.

## Recovery Signing

Recovery-generated artifacts may require Recovery Signing Authority as defined by the Bootstrap Security Model.

Examples:

```text
Recovery netboot images
Recovery manifests
Recovery reports
Recovery audit packages
```

Recovery signing is not runtime signing.

Recovery signing must be auditable.

## Recovery Completion

Recovery is complete only when:

```text
Integrity is verified
Policy is verified
Audit is imported or continuity status is explicit
Required reconciliation is complete
Required services are restored or explicitly degraded
Unsafe state is quarantined
Recovery report is generated where required
```

If these conditions are not satisfied, the system remains degraded, quarantined, failed, or unknown.

## Recovery Failure

Recovery failure must be explicit.

Recovery failure may result in:

```text
Restricted operation
Read-only operation
Recovery shell with limited authority
Supervisor recovery mode
Manual review
Full stop
```

Recovery failure must not silently become normal operation.

## Compatibility Recovery

Compatibility environments must be recovered as objects.

Legacy assumptions must not weaken native recovery rules.

If a compatibility environment cannot be reconciled safely, it must remain quarantined or disabled.

## Initial Recovery Sprint Scope

Recovery Sprint 1 should define:

```text
Recovery state enum
Recovery authority definition
Recovery object metadata
Recovery audit event identifiers
Reconciliation result enum
Basic journal replay model
Basic quarantine transitions
Basic tests
```

Recovery Sprint 1 should not implement:

```text
Distributed recovery
Automated cluster recovery
Complex policy language
Full ZFS integration
Remote recovery federation
```

## Required Tests

Initial tests should verify:

```text
Recovery authority does not grant runtime authority
Unknown recovery state fails closed
Reconciliation mismatch produces explicit result
Quarantine blocks authority-bearing operations
Journal replay is audited
Recovery does not restore revoked capability
Storage corruption triggers quarantine
Service restart does not imply reconciliation
Bootstrap recovery import requires reconciliation
Recovery completion requires audit status
```

## Required Future Work

- Define recovery object wire format.
- Define recovery manifest schema.
- Define recovery report schema.
- Define journal replay metadata.
- Define reconciliation API.
- Define recovery audit import format.
- Define integration with Storage and Persistence Model.
- Define integration with Memory and Data Movement Model.
- Define manual recovery authority rules.

## Summary

Recovery restores trust by proving consistency, not by assuming it.

The central rules are:

```text
Recovery restores trust.
Recovery does not restore authority.
Recovery is incomplete until reconciliation completes.
When trust cannot be established, quarantine beats execution.
```

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/design-principles.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/design-principles.md -->
### Integrated source: `docs/architecture/design-principles.md`

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

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/directory-service.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/directory-service.md -->
### Integrated source: `docs/architecture/directory-service.md`

# Atarix Directory Service Architecture

## Purpose

The Directory Service provides name-to-object resolution for Atarix.

Conceptually:

```text
human-readable name
  -> directory binding
  -> object identity
  -> object resolution
```

The Directory Service is not the object system, the capability system, or the mailbox system.

## Responsibilities

The Directory Service is responsible for:

- Registering name bindings.
- Looking up names.
- Removing stale bindings.
- Managing aliases.
- Enumerating namespace entries.
- Supporting service discovery.
- Supporting namespace watch notifications.
- Supporting future distributed directory caches.

## Non-Responsibilities

The Directory Service does not grant permissions, manage capabilities, schedule tasks, route messages, validate ring security, create objects, or destroy objects.

Security and authority checks remain the responsibility of the Security, Authority, Capability, and Object models.

## Security Rule

Names are not capabilities.

Lookup success does not imply access.

Access still requires ring validation, capability validation, and object-state validation.

## Binding Lifetime

When an object is destroyed, directory bindings to that object become stale and must be removed or marked stale.

Destroyed object identities shall not be reused to satisfy stale bindings.

<!-- AEMS-MIGRATED-SOURCE: docs/architecture/pillars.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/architecture/pillars.md -->
### Integrated source: `docs/architecture/pillars.md`

# Atarix Architectural Pillars

## Purpose

This document defines the architectural pillars upon which the Atarix operating system is built.

All subsystems shall identify which pillars they depend upon, which pillars they modify, and which pillars they must not violate.

## Foundational Pillars

- Object Model
- Namespace Model
- Security Model
- Authority Model

## Core Runtime Pillars

- Capability Model
- Mailbox Model
- Lifecycle Model
- State Model
- Resource Model

## Operational Pillars

- Error Model
- Audit Model
- Time Model
- Versioning Model
- Policy Model

## Engineering Pillars

- Portability Model
- Test Model
- Development Process

## Core Rule

Architecture defines implementation. Implementation shall not silently redefine architecture.

<!-- AEMS-MIGRATED-SOURCE: docs/boot-sequence-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/boot-sequence-v1.md -->
### Integrated source: `docs/boot-sequence-v1.md`

# ATARIX Boot Sequence v1

## Status

Draft boot architecture specification.

This document defines the expected power-on, reset, supervisor, FPGA, ROM monitor, discovery, and handoff sequence for ATARIX Rev A through Rev C.

## Purpose

The boot sequence must make the system recoverable, observable, and debuggable from the first hardware revision onward.

A failed boot should produce useful diagnostic information rather than a silent dead board.

## Design Goals

1. Bring up Rev A with minimal dependencies.
2. Permit supervisor-assisted recovery.
3. Permit FPGA configuration before CPU release in later revisions.
4. Keep ROM monitor entry deterministic.
5. Support discovery and capability initialization.
6. Allow safe fallback to golden ROM or recovery mode.

## Boot Actors

Primary boot participants:

```text
Supervisor Controller
FPGA Fabric
W65C816 CPU
ROM Monitor
UART Console
Future Network Service
```

## Boot Phases

```text
Phase 0 Power Stable
Phase 1 Supervisor Start
Phase 2 Fabric Preparation
Phase 3 CPU Reset Release
Phase 4 ROM Monitor Start
Phase 5 Hardware Diagnostics
Phase 6 Discovery Scan
Phase 7 Service Initialization
Phase 8 Loader or Kernel Handoff
```

---

## Phase 0: Power Stable

Power rails must settle before logic is released.

Required checks:

- +5V within tolerance.
- +3.3V within tolerance.
- Clock source stable.
- Reset asserted.

If power is unstable, the supervisor must hold the CPU and FPGA in reset.

---

## Phase 1: Supervisor Start

The supervisor controller starts before the W65C816.

Responsibilities:

- Validate power-good state.
- Select normal ROM or golden ROM.
- Check recovery request input.
- Initialize fault log buffer.
- Prepare reset sequencing.

Recovery triggers may include:

```text
Manual recovery switch
Previous boot failure
ROM checksum failure
Supervisor command
Watchdog timeout
```

---

## Phase 2: Fabric Preparation

Rev A may not require FPGA fabric.

Rev C and later should configure the FPGA before CPU release when the FPGA owns address decode, interrupt routing, or other critical services.

Fabric preparation includes:

- Load FPGA bitstream.
- Verify configuration done signal.
- Initialize fabric control registers.
- Disable DMA by default.
- Disable accelerators by default.
- Enable only required boot services.

Default security state:

```text
DMA disabled
Accelerators disabled
Discovery read-only
Interrupt routing minimal
Supervisor mailbox enabled
```

---

## Phase 3: CPU Reset Release

After required services are ready, the supervisor releases the W65C816 reset line.

The CPU begins execution from the reset vector in ROM.

Rev A reset path:

```text
Power Stable -> CPU Reset Release -> ROM Monitor
```

Rev C reset path:

```text
Power Stable -> Supervisor -> FPGA Config -> CPU Reset Release -> ROM Monitor
```

---

## Phase 4: ROM Monitor Start

The ROM monitor is the first W65C816 program executed.

Initial monitor tasks:

- Enter known processor mode.
- Initialize stack.
- Initialize direct page.
- Initialize UART.
- Print boot banner.
- Report ROM version.
- Report reset reason if available.

Minimum banner fields:

```text
ATARIX ROM Monitor
ROM Version
Reset Reason
CPU Mode
Detected RAM
```

---

## Phase 5: Hardware Diagnostics

Early diagnostics should be conservative.

Recommended checks:

1. ROM checksum.
2. Basic RAM pattern test.
3. UART loopback or transmit test.
4. Supervisor mailbox ping.
5. Fabric status read if present.

Diagnostics should be skippable by command or jumper once hardware is stable.

---

## Phase 6: Discovery Scan

The monitor scans known discovery sources.

Discovery sources may include:

```text
Fixed ROM tables
Supervisor-provided tables
FPGA discovery block
Slot discovery ROMs
Device-local EEPROMs
```

The monitor must:

1. Verify discovery magic.
2. Validate version.
3. Read fixed identity header.
4. Iterate TLV records.
5. Register devices in a simple device table.
6. Ignore unknown optional records.

See:

```text
docs/discovery-rom-format.md
```

---

## Phase 7: Service Initialization

After discovery, the monitor initializes boot-critical services.

Candidate service order:

1. UART console.
2. Supervisor mailbox.
3. Timer service.
4. Interrupt controller.
5. Network service if present.
6. Storage service if present.

DMA must remain disabled until a valid capability model exists for the requested transfer.

---

## Phase 8: Loader or Kernel Handoff

The monitor may enter one of several modes:

```text
Interactive Monitor
Serial Loader
Network Loader
Storage Boot
Kernel Handoff
Recovery Mode
```

Kernel handoff requires:

- Memory map.
- Device table.
- Capability root or boot token.
- Interrupt state.
- Loader parameters.

## Recovery Mode

Recovery mode should provide:

- UART console.
- ROM inspection.
- RAM test.
- Firmware upload.
- FPGA reload if supported.
- Supervisor fault log access.

Recovery mode must avoid enabling DMA, accelerators, or untrusted device services by default.

## Fault Reporting

Boot faults should include:

```text
Boot Phase
Fault Code
Reset Reason
Supervisor Status
Fabric Status
Last Successful Step
```

Fault data should be visible through:

- UART monitor.
- Supervisor mailbox.
- Fault log command.

## Boot Security Rules

Default boot posture:

```text
Least privilege
No DMA by default
No accelerator execution by default
No unverified service startup
Supervisor root authority
```

## Open Questions

- Final ROM monitor command set.
- Whether network boot appears in Rev B or later.
- Exact supervisor fault-log layout.
- Whether golden ROM is physically separate or bank-selected.
- Whether the FPGA bitstream is supervisor-loaded or ROM-loaded.
- Kernel handoff data-structure format.

## Design Rule

The machine must always have a path back to a human-visible monitor prompt unless hardware itself has failed.

<!-- AEMS-MIGRATED-SOURCE: docs/bus-architecture.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/bus-architecture.md -->
### Integrated source: `docs/bus-architecture.md`

# ATARIX Bus Architecture

## Status

Draft architectural decision record.

This document records the selected direction for the ATARIX card and backplane architecture.

## Design Decision

ATARIX will use a hybrid modular bus architecture inspired by Sun workstation expansion buses and Apple NuBus-style self-description.

The backplane should not simply expose the raw W65C816 bus to every slot.

Instead, the W65C816 bus remains local to the CPU card where possible, while the backplane exposes a structured fabric-oriented interface for devices, services, accelerators, and future CPU modules.

## Influences

### Sun SPARCstation-Style Expansion

ATARIX should take inspiration from Sun workstation expansion systems such as SPARCstation-class modular I/O.

Relevant ideas:

- Workstation-oriented modular expansion.
- Stable card/backplane mechanical model.
- Clean separation between host processor and expansion devices.
- Device slots as first-class architectural participants.

### NuBus-Style Self Description

ATARIX should also take inspiration from NuBus-style cards.

Relevant ideas:

- Cards identify themselves.
- Cards describe resources.
- Driver matching can rely on structured metadata.
- Expansion devices are more than simple fixed-address peripherals.

ATARIX discovery is defined separately in:

```text
docs/discovery-rom-format.md
```

## Hybrid ATARIX Approach

The selected model is:

```text
W65C816 local bus
        |
CPU card local glue / bridge
        |
ATARIX fabric interface
        |
Active backplane
        |
Service cards / fabric cards / accelerator cards
```

The CPU card may expose selected bus-derived transactions, but the backplane should prefer a controlled fabric interface rather than every raw CPU signal.

## Why Not a Raw 65C816 Backplane?

A raw CPU bus backplane is simple, but it creates long-term problems:

- Poor scaling to multiple CPU cards.
- Difficult signal integrity.
- Weak fault isolation.
- Difficult DMA mediation.
- No natural discovery model.
- No clean authority boundary.
- Harder accelerator integration.

ATARIX is intended to become more than a single-board 65C816 machine, so the bus should not be designed around the simplest possible first revision only.

## Why Not a Fully Abstract Fabric Only?

A fully abstract fabric would lose the value of the W65C816 architecture and make early bring-up harder.

ATARIX should preserve:

- Observable CPU behavior.
- Simple Rev A diagnostics.
- ROM monitor access.
- Direct hardware understanding.

Therefore, the CPU-local bus remains important, especially in Rev A and Rev B.

## Layered Bus Model

### Layer 0: CPU Local Bus

Scope:

- CPU card only.

Contains:

- W65C816 address bus.
- W65C816 data bus.
- CPU control signals.
- Local ROM or boot mapping.
- Local SRAM if used.
- Local glue logic.

### Layer 1: CPU Bridge Interface

Scope:

- Boundary between CPU card and system fabric.

Responsibilities:

- Translate CPU bus cycles into fabric transactions.
- Isolate CPU card timing from backplane timing.
- Provide wait-state or ready signaling.
- Support debug visibility.

### Layer 2: ATARIX Fabric Interface

Scope:

- Backplane-level logical interface.

Responsibilities:

- Addressed transactions.
- Mailbox delivery.
- Interrupt routing.
- Discovery access.
- DMA request mediation.
- Capability-aware access control.

### Layer 3: Active Backplane

Scope:

- Slot-to-slot system interconnect.

Responsibilities:

- Slot identification.
- Reset distribution.
- Clock distribution.
- Power distribution.
- Fabric routing.
- Fault containment.

### Layer 4: Service and Accelerator Modules

Scope:

- Cards attached to the backplane.

Examples:

- Network card.
- Storage card.
- FPGA service card.
- ARM compute module.
- Future accelerator.
- Future secondary CPU card.

## Card Identity

Every nontrivial card should provide discovery information.

Required identity fields:

- Vendor ID.
- Device ID.
- Device class.
- Revision.
- Capability records.
- Optional driver binding string.

## Transaction Model

The fabric interface should support these transaction classes:

```text
Register Read
Register Write
Mailbox Send
Mailbox Receive
Discovery Read
DMA Request
Interrupt Signal
Fault Report
```

Early revisions may implement only a subset.

## CPU Card Role

The primary CPU card is not just a passive bus master.

It is a participant in a larger system fabric.

Responsibilities:

- Boot monitor execution.
- Service discovery.
- Fabric register access.
- Mailbox participation.
- Capability broker participation in later revisions.

## FPGA Role

The FPGA is the natural fabric bridge and enforcement assistant.

Responsibilities may include:

- Address decode.
- Transaction routing.
- Interrupt routing.
- Mailbox handling.
- Discovery table exposure.
- DMA mediation.
- Fault logging.

The FPGA should not be treated as an undocumented black box.

## Backplane Design Implications

The backplane should include signal groups for:

- Power.
- Ground.
- Reference clock.
- Reset.
- Slot identification.
- Interrupt or event signaling.
- Fabric data path.
- Fabric control path.
- Discovery access.
- Debug and recovery.

The exact pinout is deferred to:

```text
docs/backplane-pinout-v1.md
```

## CPU Card Design Implications

The CPU card should include:

- Local W65C816 bus.
- Local bridge or glue logic.
- Debug access.
- Reset control.
- Clock input or local clock selection.
- Discovery record.
- Fabric-facing interface.

The exact pinout is deferred to:

```text
docs/cpu-card-pinout-v1.md
```

## Security Implications

Because the backplane is not a raw shared CPU bus, ATARIX can enforce stronger rules:

- Devices are not automatically trusted.
- DMA requests can be mediated.
- Discovery can declare required capabilities.
- Faults can be isolated per slot.
- Accelerators can receive explicit memory grants.

See:

```text
docs/capability-model.md
docs/dma-engine-v1.md
```

## Development Phasing

### Rev A

Raw W65C816 system for bring-up.

### Rev B

CPU card preparation and early bridge concepts.

### Rev C

FPGA-mediated fabric interface begins.

### Rev D

Multi-CPU and mediated resource experiments.

### Rev E

Accelerator and heterogeneous compute framework.

## Design Rule

Keep the W65C816 bus local where possible.

Expose a structured, discoverable, fabric-oriented interface to the backplane.

This preserves early debuggability while allowing ATARIX to grow into a modular workstation architecture.

<!-- AEMS-MIGRATED-SOURCE: docs/capability-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/capability-model.md -->
### Integrated source: `docs/capability-model.md`

# ATARIX Capability Model

## Status

Draft security architecture specification.

This document defines the capability system used to control access to memory, DMA, mailboxes, devices, FPGA services, future 32-bit / 64-bit CPU cards, memory services, storage services, and accelerators.

## Design Philosophy

ATARIX assumes that attachment does not imply trust.

A device, accelerator, CPU card, or service module does not automatically receive unrestricted access merely because it is physically connected.

Instead, access is granted through explicit capabilities.

## Data Model

Capability records follow:

```text
docs/data-model-and-endianness.md
```

All multi-byte fields are little-endian.

Capability identifiers, resource identifiers, memory object identifiers, revocation identifiers, and persistent audit references should use u64.

The W65C816's u24 CPU-local address space must not constrain capability records or fabric-resource identity.

## Goals

1. Prevent unrestricted DMA.
2. Permit safe accelerator integration.
3. Support multi-CPU experimentation.
4. Enable revocation.
5. Remain implementable on a W65C816 platform.
6. Avoid requiring heavyweight cryptography for local operation.
7. Support fabric resources and memory objects larger than the W65C816 native address space.
8. Support future 32-bit and 64-bit CPU cards without changing the authority model.

## Definition

A capability is a structured authorization record describing:

- Who owns the capability.
- What resource is controlled.
- What operations are permitted.
- How long the grant remains valid.
- Whether the grant can be delegated.
- How it can be revoked or audited.

## Capability Classes

### Memory Capability

Controls access to a memory range, memory window, or memory-service object.

Permissions:

```text
Read
Write
Execute
Map Window
DMA Visible
```

### DMA Capability

Controls DMA engine access.

Permissions:

```text
Read Memory
Write Memory
Bidirectional
Submit Descriptor
Manage Queue
```

### Mailbox Capability

Controls mailbox endpoints.

Permissions:

```text
Send
Receive
Administrative
Privileged Command
```

### Device Capability

Controls access to device registers.

Examples:

```text
UART
Network Controller
Storage Controller
Timer Device
```

### Fabric Capability

Controls FPGA fabric services.

Examples:

```text
Discovery Access
Interrupt Configuration
DMA Configuration
Mailbox Routing
Fabric Register Access
```

### Memory Service Capability

Controls managed memory resources that may exceed CPU-local address sizes.

Examples:

```text
Allocate Object
Map Window
Read Object
Write Object
Pin Object For DMA
Revoke Mapping
```

### Storage Service Capability

Controls persistent storage resources.

Examples:

```text
Read Extent
Write Extent
Create Object
Delete Object
Snapshot Object
```

### Accelerator Capability

Controls execution on accelerator resources.

Examples:

```text
Submit Job
Read Results
Manage Queue
Grant DMA Window
```

## Capability Record Format v1

Version 1 capability record:

```text
Offset  Size  Field
+00     2     Capability Class, u16
+02     2     Capability Flags, u16
+04     8     Capability ID, u64
+0C     8     Owner ID, u64
+14     8     Resource ID, u64
+1C     8     Resource Offset, u64
+24     8     Resource Length, u64
+2C     8     Expiration Time or Monotonic Expiration, u64
+34     8     Parent Capability ID, u64, or zero
+3C     8     Revocation Generation, u64
+44     4     Reserved, must be zero
```

Minimum size:

```text
72 bytes
```

The larger record is intentional. Capabilities are security-critical and should not be constrained by early CPU-local addressing limits.

## Compact Capability References

Compact protocols may carry a u64 Capability ID rather than embedding the full record.

Examples:

```text
Mailbox extended header
DMA fabric descriptor
Fault log entry
Supervisor diagnostic record
```

The full capability record may live in:

- Kernel memory.
- Fabric memory.
- Supervisor-managed tables.
- Memory-service metadata.
- A future capability broker.

## Capability Flags

```text
Bit 0  Read
Bit 1  Write
Bit 2  Execute
Bit 3  Configure
Bit 4  Delegate
Bit 5  Revoke
Bit 6  Persistent
Bit 7  Audit Required
Bit 8  DMA Visible
Bit 9  Map Window
Bit 10 Privileged Operation
Bit 11 Supervisor Visible
Bits 12-15 Reserved
```

## Ownership Model

Capabilities belong to an owner.

Owner identifiers should be u64.

Owner types:

```text
W65C816 CPU Card
Future 32-bit CPU Card
Future 64-bit CPU Card
Supervisor
Fabric Service
Network Service
Storage Service
Memory Service
Accelerator
```

A future secondary CPU card does not automatically receive Ring 0 or root authority merely because it is physically installed.

## Resource Model

Resources should be identified by u64 Resource IDs.

Resource examples:

```text
CPU-local aperture
Memory-service object
Storage extent
Mailbox endpoint
DMA channel
Interrupt source
Fabric register block
Accelerator queue
Device register window
```

Resource offsets and lengths should be u64 when referring to persistent or large resources.

Small local resources may use narrower fields in local protocols, but their capability records should remain u64-capable.

## Delegation

Capabilities may optionally be delegated.

Delegated capabilities:

- Cannot exceed the authority of the parent capability.
- Maintain a reference to the parent grant.
- Can be revoked by the parent owner.
- Should receive their own u64 Capability ID.

## Revocation

Revocation is a core architectural feature.

A revoked capability becomes invalid immediately.

Future implementations may use:

- Capability tables.
- Generation counters.
- Revocation lists.
- Per-resource revocation generations.
- Supervisor-visible quarantine records.

The Revocation Generation field allows stale cached capability references to be detected.

## Expiration

Expiration should use u64.

Possible encodings:

```text
u64 seconds since epoch
u64 monotonic tick deadline
u64 job-completion token
0 = no automatic expiration
```

The owning subsystem must specify which interpretation is used.

## Memory Grants

Example:

```text
Owner:       DMA Engine
Resource:    Memory Object #42
Offset:      0x0000000200000000
Length:      0x0000000000010000
Permissions: Read + Write + DMA Visible
Expiration: Job Completion
```

This example intentionally uses offsets larger than the W65C816 native u24 address space.

## DMA Grants

DMA operations require:

1. DMA capability.
2. Source-resource capability.
3. Destination-resource capability.
4. Valid ownership chain.
5. Ring/security policy allowing the request.

The DMA engine should reject requests lacking proper authorization.

See:

```text
docs/dma-engine-v1.md
```

## Mailbox Grants

Mailbox endpoints may require explicit authorization.

Examples:

```text
CPU -> Supervisor
CPU -> Fabric
CPU -> Accelerator
Service -> Memory Service
```

Mailbox messages that reference capabilities should use u64 Capability IDs.

See:

```text
docs/mailbox-protocol-v1.md
```

## Accelerator Grants

Accelerators should never receive unrestricted memory visibility.

Instead:

1. Job submitted.
2. Memory buffers granted.
3. Execution authorized.
4. Results returned.
5. Capabilities revoked.

Accelerator grants should be narrow, time-limited where practical, and auditable.

## Discovery Integration

Discovery records may advertise:

- Required capability classes.
- Capability restrictions.
- Administrative requirements.
- Resource ID requirements.
- Whether DMA access must be granted explicitly.

See:

```text
docs/discovery-rom-format.md
```

## Audit Events

Future systems may log:

- Capability creation.
- Capability delegation.
- Capability revocation.
- Capability violations.
- Capability expiration.
- Capability use by DMA or accelerator engines.

Audit records should use u64 timestamps and u64 capability references.

## Future Hardware Assistance

Future FPGA revisions may assist with:

- DMA validation.
- Capability lookup.
- Access enforcement.
- Fault reporting.
- Revocation-generation checks.
- Resource-window bounds checking.

The FPGA acts as an enforcement assistant rather than the only trusted authority.

## Security Interaction With Rings

Privilege rings define execution authority.

Capabilities define resource authority.

Both are required.

Example:

```text
Ring 1 service + valid storage capability
    -> may operate assigned storage resource

Ring 3 program + no capability
    -> denied

Ring 2 driver + expired DMA capability
    -> denied
```

See:

```text
docs/security.md
```

## Design Rule

The supervisor remains the root management and recovery authority.

The FPGA assists enforcement.

Devices receive only the minimum authority necessary to perform their tasks.

Capability identity and resource identity are not limited by the W65C816 native address space.

## Open Questions

- Final capability table structure.
- Persistence across reboot.
- Distributed capability ownership.
- Hardware acceleration of validation.
- Formal capability serialization format.
- Cryptographic signing requirements for remote services.
- Whether 128-bit object identifiers will be required for distributed systems.
- Whether expiration uses wall-clock time, monotonic ticks, or both.

<!-- AEMS-MIGRATED-SOURCE: docs/capability-record-format-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/capability-record-format-v1.md -->
### Integrated source: `docs/capability-record-format-v1.md`

# Capability Record Format v1

Status: Draft v1

## Purpose

Capability Records define authorized operations against resources and services discovered through the Enumeration Fabric.

Discovery describes what exists.

Capabilities authorize what may be done.

Rings constrain who may exercise that authority.

## Architectural Chain

```text
Identity
    identifies

Discovery
    describes

Trust
    evaluates

Capabilities
    authorize

Rings
    constrain

Directory
    locates

Transport
    operates
```

## Non-Goals

Capability Records do not:

- identify hardware
- enumerate resources
- define physical addresses
- define IRQ numbers
- define DMA channels
- replace supervisor safety policy
- override Ring -2 authority

## Handle Binding

Capabilities target handles produced by Discovery and later resolved by Directory Services.

A capability target is a handle, not an address.

## Capability Record Layout

All numeric fields are little-endian.

All records are 8-byte aligned.

```text
Offset  Size  Field
------  ----  ----------------------
0x00    4     Capability ID
0x04    4     Operation ID
0x08    8     Target Handle
0x10    2     Minimum Ring
0x12    2     Maximum Ring
0x14    4     Flags
0x18    8     Delegation Mask
0x20    8     Expiration / Lifetime
0x28    8     Issuer Handle
0x30    4     Trust Requirement
0x34    4     Reserved
```

Total size:

```text
56 bytes
```

## Capability ID

Capability ID identifies the capability class.

Examples:

```text
READ_HEALTH
READ_COUNTERS
CAPTURE_TRACE
CONTROL_MAILBOX
PERFORM_DMA
BOOT_FROM_RESOURCE
ENTER_RECOVERY
DRIVE_PATTERN_GENERATOR
```

A formal capability ID registry may be defined later.

## Operation ID

Operation ID identifies the specific operation authorized by this record.

Examples:

```text
READ
WRITE
CONTROL
RESET
CAPTURE
STIMULATE
BOOT
RECOVER
PROVISION
```

## Target Handle

Target Handle identifies the resource or service to which the capability applies.

The handle uses the Discovery ROM handle model.

## Minimum Ring and Maximum Ring

Rings constrain who may exercise the capability.

Initial rings:

```text
-2  Hardware Safety / Supervisor Authority
-1  Fabric / Platform Firmware Authority
 0  Kernel / Executive Authority
 1  Driver / Service Authority
 2  User Service Authority
 3  Application Authority
```

Encoding recommendation:

```text
Ring -2 = 0xFFFE
Ring -1 = 0xFFFF
Ring  0 = 0x0000
Ring  1 = 0x0001
Ring  2 = 0x0002
Ring  3 = 0x0003
```

## Flags

Initial flags:

```text
Bit 0   Read allowed
Bit 1   Write allowed
Bit 2   Control allowed
Bit 3   Delegation allowed
Bit 4   Revocation supported
Bit 5   Time limited
Bit 6   Development only
Bit 7   Recovery only
Bit 8   Destructive operation
Bit 9   Requires supervisor approval
Bits 10-31 reserved
```

## Delegation Mask

Delegation Mask describes which lower rings, if any, may receive delegated access.

A zero mask means no delegation.

## Expiration / Lifetime

Expiration may represent:

- zero for no expiration
- monotonic deadline
- policy-defined lifetime
- provisioning epoch

Exact interpretation is policy-defined.

## Issuer Handle

Issuer Handle identifies the provider, supervisor, firmware, or authority that issued the capability.

Issuer authority must be validated by trust and ring policy.

## Trust Requirement

Trust Requirement indicates the minimum trust state required before the capability may be accepted.

Initial values:

```text
0  UNTRUSTED
1  IDENTIFIED
2  VALIDATED
3  TRUSTED
4  PRIVILEGED
5  RESERVED
6  RESERVED
7  REVOKED
```

A capability requiring TRUSTED shall not be accepted from a merely IDENTIFIED entity.

A REVOKED entity shall not grant usable capabilities.

## Security Rules

```text
Discovery describes.
Capabilities authorize.
Rings constrain authority.
Supervisor safety authority overrides capability grants.
```

A valid capability record is not sufficient by itself.

The platform must also evaluate:

- identity
- discovery validity
- trust state
- ring authority
- supervisor policy
- recovery state
- revocation state

## Homebrew Default Policy

Homebrew cards may advertise capabilities, but default platform policy should reject or restrict dangerous operations.

Default denials:

```text
DMA
Boot authority
Recovery authority
Supervisor authority
Memory-controller authority
Privileged instrumentation stimulus
```

## Related Documents

- discovery-rom-format-v1.md
- trust-model-v1.md
- ADR-RING-SECURITY-MODEL.md
- resource-type-registry-v1.md
- service-id-registry-v1.md
- service-directory-format-v1.md

<!-- AEMS-MIGRATED-SOURCE: docs/card-health-model-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/card-health-model-v1.md -->
### Integrated source: `docs/card-health-model-v1.md`

# Card Health Model v1

Status: Draft v1

## Purpose

This document defines the common ATARIX card health model.

The health model provides a shared vocabulary for supervisors, validation tools, provisioning tools, monitors, firmware, and operating systems.

The health model answers:

```text
Is this card safe?
Is this card ready?
If not, why?
```

## Architectural Principles

```text
Measurement must not depend on the thing being measured.
```

```text
Every subsystem shall be:

Discoverable
Observable
Testable
Instrumentable
Recoverable
```

## Overall Health States

```text
ABSENT
PRESENT_UNVALIDATED
IDENTITY_VALID
DISCOVERY_VALID
RESTRICTED
READY
WARNING
FAULTED
SAFE_MODE
RECOVERY_MODE
```

## State Definitions

### ABSENT

No card is detected.

### PRESENT_UNVALIDATED

A card appears physically present, but identity and health validation are incomplete.

### IDENTITY_VALID

Identity EEPROM validation succeeded.

### DISCOVERY_VALID

Discovery ROM validation succeeded.

### RESTRICTED

The card is valid but policy restricts participation.

Common causes:

- homebrew policy
- development mode
- insufficient trust
- missing capability authorization
- ring restriction

### READY

The card has passed required validation and is available for normal participation.

### WARNING

The card is operational but has a warning condition.

Examples:

- elevated temperature
- rail margin warning
- degraded link
- missing optional service

### FAULTED

The card has a fault that prevents normal operation.

### SAFE_MODE

The card is held in a safe operating state pending diagnosis or policy decision.

### RECOVERY_MODE

The card is in a recovery or provisioning workflow.

## Health Domains

Each card may report health in multiple domains.

Required domains where applicable:

```text
Identity Health
Discovery Health
Power Health
Current Health
Thermal Health
Clock Health
Reset Health
Watchdog Health
Communication Health
Fabric Health
CPU Health
Instrumentation Health
```

## Domain States

Recommended per-domain states:

```text
UNKNOWN
NOT_PRESENT
OK
WARNING
CRITICAL
FAULT
RECOVERING
DISABLED_BY_POLICY
```

## Power Health

Power health may include:

- input voltage present
- regulator output valid
- rail sequencing complete
- under-voltage state
- over-voltage state
- power-good state

Common rails:

```text
VIN_BACKPLANE
V5_CARD
V3P3_CARD
V2P5_CARD
V1P8_CARD
V1P2_CARD
VCORE_CARD
VBAT_OR_RTC
```

## Current Health

Current health may include:

- input current
- per-rail current
- inrush event count
- over-current state
- fuse or eFuse state
- power budget class

## Thermal Health

Thermal health may include:

- board temperature
- regulator temperature
- FPGA temperature
- CPU temperature
- memory temperature
- warning threshold
- critical threshold

## Clock Health

Clock health may include:

- reference clock present
- fabric clock present
- CPU clock present
- PLL lock state
- frequency within tolerance
- missing-clock state

## Reset and Watchdog Health

Reset and watchdog health may include:

- reset asserted state
- reset reason
- reset hold reason
- watchdog enabled
- watchdog timeout count
- last watchdog event

## Communication Health

Communication health may include:

- sideband bus state
- mailbox state
- fabric link state
- timeout count
- transaction error count

## CPU Health

CPU cards additionally use the CPU Observability Contract.

Common CPU health indicators:

```text
CPU_POWER_GOOD
CPU_CLOCK_GOOD
CPU_RESET_ASSERTED
CPU_RESET_RELEASED
CPU_FETCHING
CPU_EXECUTING
CPU_ENUMERATED
CPU_MONITOR_RUNNING
CPU_OS_RUNNING
CPU_FAULTED
```

## Instrumentation Health

Instrumentation health may include:

- logic analyzer available
- trace buffer available
- protocol decoder available
- counter bank available
- trigger router available
- capture active
- capture fault

## Event Reporting

Health transitions should generate supervisor event log records.

Recommended fields:

```text
timestamp_us
slot_or_card_id
health_domain
previous_state
new_state
reason_code
raw_status
```

## Recovery Rule

Recovery must not depend on the failed subsystem.

The supervisor must retain enough visibility to distinguish:

- absent card
- invalid identity
- invalid discovery
- power fault
- thermal fault
- clock fault
- communication fault
- policy restriction
- revoked trust

## Ring Security Notes

Health observation is not the same as health control.

Passive health visibility may be delegated to lower rings by policy.

Health control, recovery entry, provisioning entry, and reset authority generally remain Ring -2 or Ring -1 functions.

## Related Documents

- supervisor-observability-contract-v1.md
- cpu-observability-contract-v1.md
- fpga-instrumentation-testbench-services-v1.md
- trust-model-v1.md
- ADR-RING-SECURITY-MODEL.md

<!-- AEMS-MIGRATED-SOURCE: docs/card-identity-eeprom.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/card-identity-eeprom.md -->
### Integrated source: `docs/card-identity-eeprom.md`

# ATARIX Card Identity EEPROM

## Status

Draft architecture specification.

This document defines the small nonvolatile identity device that should be present on each ATARIX card, and the minimum independent management objects expected on every card.

## Purpose

Each ATARIX card should include a small identity EEPROM that allows the supervisor, boot firmware, or fabric controller to identify the card installed in a slot before the main card logic is fully initialized.

The identity EEPROM provides a stable, low-power, early-boot source of information for:

- Card identity.
- Vendor and product identifiers.
- Hardware revision.
- Serial number.
- Board capabilities.
- Power requirements.
- Required firmware or bitstream versions.
- Discovery-record location.
- Manufacturing data.
- Recovery and safe-mode information.

## Historical Influences

The design is inspired by:

- NuBus declaration-ROM style self-description.
- EISA-style configuration records.
- Modern SPD EEPROMs used on memory modules.
- Server FRU inventory EEPROMs.

ATARIX should borrow the self-description idea, not copy any legacy format directly.

## Design Rule

A card should be identifiable before it is trusted.

A card should be identifiable before its main processor, FPGA, CPLD, or device logic is allowed to perform privileged fabric activity.

Attachment does not imply trust.

## Minimum Card Management Standard

Every ATARIX card should include a small independent management cluster that can be accessed before the card is fully trusted by the fabric.

These management objects should function independently from the card's main processing logic whenever practical.

A failed CPU, FPGA function, accelerator core, or service engine should not prevent the supervisor from reading identity, basic health, power, temperature, and fault state.

### Mandatory Management Objects

Every card should provide:

```text
Identity EEPROM / FRU EEPROM
Temperature Sensor
Voltage Monitor
Current Monitor
Status / Control Register
```

The status / control register should expose at minimum:

```text
READY
FAULT
DEGRADED
SERVICE_REQUEST
SELFTEST_PASS
SELFTEST_FAIL
WRITE_PROTECT_STATUS
LAST_RESET_REASON
```

### Recommended Management Objects

Most nontrivial cards should provide:

```text
Fan Tachometer
Fan Controller
Additional ADC Channels
Environmental Sensors
```

Additional ADC channels may monitor local rails, references, battery or supercapacitor rails, analog supplies, or sensor inputs.

Environmental sensors may include board temperature, inlet temperature, outlet temperature, humidity, or other card-relevant conditions.

Cards expected to dissipate significant power should treat fan tachometer, fan control, and thermal sensing as mandatory rather than merely recommended.

### Optional / High-Reliability Management Objects

Cards may provide:

```text
Secure Element
Calibration EEPROM
Management MCU
```

A secure element may support signed identity, secure boot, certificate storage, firmware authorization, or asset tracking.

A calibration EEPROM may store factory calibration, ADC calibration, temperature offsets, power-monitor calibration, board-specific tuning, and field-service data.

A management MCU may aggregate sensors, run card-local self-tests, supervise local resets, or communicate with the system supervisor.

### Independence Requirement

Each management object should fail independently where practical.

Examples:

- Identity EEPROM remains readable even if the main card logic is held in reset.
- Temperature and power monitors remain readable before card enable.
- Status / control register can report FAULT even when the main data plane is disabled.
- Write-protect state can be observed without booting the card.
- Calibration data can be read during maintenance or recovery mode.

A card that cannot report its own identity and basic health should not be granted normal operating authority.

## Relationship to Discovery Records

The identity EEPROM is not a replacement for the full discovery system.

Instead:

```text
Identity EEPROM
    Small, early, low-level, always readable where practical.

Discovery Record
    Larger, richer, versioned, fabric-visible system description.
```

The EEPROM may contain either:

1. A compact identity record directly, or
2. A pointer to a richer discovery record stored elsewhere, or
3. Both.

Possible richer discovery locations:

```text
On-card ROM
On-card flash
FPGA block RAM
Supervisor-provided table
Fabric-generated record
Boot-updated record cache
```

## Bus Interface

Recommended Rev A interface:

```text
I2C-compatible serial EEPROM
```

Rationale:

- Simple wiring.
- Low pin count.
- Easy supervisor access.
- Similar operational model to SPD EEPROMs.
- Easy read access before full card initialization.

The backplane should provide an identity-management path that can be accessed by the supervisor and/or fabric controller.

The identity-management path should also support the minimum card management objects where practical.

## Slot Addressing

The system must be able to distinguish EEPROMs and management objects by slot.

Possible approaches:

- Slot-specific I2C mux.
- Slot address pins into device address pins.
- Supervisor-controlled per-slot enable.
- Fabric-controlled identity bus arbitration.
- Per-slot management-bus isolation.

Rev A should prefer the simplest electrically reliable scheme.

## Write Policy

The identity EEPROM should contain manufacturing identity that is normally read-only in system operation.

Writable regions may be allowed for:

- Boot-generated cached discovery data.
- Calibration data.
- Board configuration.
- Field-service data.
- Last-known-good configuration pointer.

However, writes must be controlled.

Recommended policy:

```text
Manufacturing region: write-protected after programming.
System region: writable only by supervisor-authorized process.
Recovery region: writable only in explicit maintenance mode.
```

The normal operating system should not freely rewrite identity EEPROM contents.

## Identity Record v1

All multi-byte fields are little-endian and follow:

```text
docs/data-model-and-endianness.md
```

Recommended fixed header:

```text
Offset  Size  Field
+00     4     Magic: "ATID"
+04     1     Format Major Version
+05     1     Format Minor Version
+06     2     Header Length, u16
+08     2     Total Length, u16 or extended form
+0A     2     Header Checksum, u16
+0C     2     Vendor ID, u16
+0E     2     Product ID, u16
+10     2     Device Class, u16
+12     2     Device Subclass, u16
+14     2     Hardware Revision, u16
+16     2     Board Revision, u16
+18     8     Serial Number or Board ID, u64
+20     8     Capability Summary, u64
+28     8     Required Power Summary, u64
+30     8     Discovery Pointer or Resource ID, u64
+38     8     Manufacturing Timestamp, u64
+40     8     Reserved, must be zero
```

Minimum header size:

```text
72 bytes
```

The EEPROM may then contain TLV records.

## Standard TLV Records

Recommended TLV record form:

```text
Offset  Size  Field
+00     1     Type
+01     1     Flags
+02     2     Length, u16
+04     N     Value
```

For longer values, use a TLV type that explicitly contains a u32 or u64 length inside the value.

Recommended TLV types:

```text
$00 End
$01 Human-readable card name
$02 Manufacturer name
$03 Product name
$04 Firmware compatibility
$05 FPGA bitstream compatibility
$06 Power requirements
$07 Clock requirements
$08 Reset requirements
$09 Discovery record location
$0A Required capability classes
$0B Supported safe modes
$0C Diagnostic entry points
$0D Manufacturing data
$0E Field-service data
$0F Calibration data
$10 Boot cache pointer
$11 Management object inventory
$12 Thermal sensor description
$13 Power monitor description
$14 Fan controller description
$15 Environmental sensor description
$16 Secure element description
$17 Calibration EEPROM description
$80-$FF Vendor-specific
```

## Required Fields

Every card identity EEPROM should provide at minimum:

```text
Magic
Format version
Vendor ID
Product ID
Device class
Hardware revision
Serial number or board ID
Capability summary
Power requirement summary
Management object inventory
```

## Capability Summary

The capability summary is not a security grant.

It is only an early-boot hint describing what the card claims to support.

Actual access still requires capability authorization.

Examples:

```text
DMA capable
Mailbox capable
Interrupt capable
Memory service capable
Storage service capable
Accelerator capable
Supervisor visible
Fabric configuration required
Management sensor capable
Secure identity capable
Calibration data present
```

## Boot Flow

Recommended boot sequence:

```text
1. Supervisor powers identity and management bus.
2. Supervisor scans slots.
3. Supervisor reads each identity EEPROM.
4. Supervisor validates checksum and version.
5. Supervisor reads mandatory management objects.
6. Supervisor records slot inventory and health baseline.
7. Fabric controller receives slot identity summary.
8. Boot firmware queries fabric identity table.
9. Full discovery records are read or generated.
10. Capability broker assigns permissions.
11. Devices become usable only after authorization.
```

## Identity Fabric

The identity fabric is the early-boot inventory path that collects identity information from cards.

It may be implemented by:

- Supervisor firmware.
- Fabric-controller logic.
- A dedicated identity bus.
- A combination of supervisor and fabric support.

The identity fabric should expose a stable inventory table to boot firmware and diagnostics.

## Security Considerations

Identity is not authority.

A card may claim capabilities in its EEPROM, but the system must not grant access solely because the EEPROM says so.

Security rules:

1. Treat EEPROM data as untrusted until validated.
2. Verify checksums and versions.
3. Apply policy before granting capabilities.
4. Do not allow unrestricted EEPROM writes.
5. Record identity mismatches in supervisor logs.
6. Quarantine cards with invalid or inconsistent identity data where appropriate.
7. Treat sensor data as advisory unless validated by policy.
8. Secure elements may strengthen identity but do not replace capability authorization.

## Diagnostics

The supervisor and firmware monitor should support commands similar to:

```text
IDENT LIST
IDENT SHOW <slot>
IDENT RAW <slot>
IDENT VERIFY <slot>
IDENT CACHE REFRESH
HEALTH SHOW <slot>
SENSOR SHOW <slot>
POWER SHOW <slot>
FAN SHOW <slot>
CALIBRATION SHOW <slot>
```

## Relationship to Testing

Testing should include:

- Valid identity EEPROM parsing.
- Invalid checksum handling.
- Unknown version handling.
- Missing EEPROM handling.
- Conflicting discovery-record handling.
- Write-protection checks.
- Slot inventory stability.
- Mandatory management-object detection.
- Temperature sensor readout.
- Voltage and current monitor readout.
- Fan tachometer and fan controller behavior where present.
- Calibration EEPROM readout where present.
- Secure-element presence and failure handling where present.

See:

```text
docs/testing-strategy.md
```

## Design Principle

The identity EEPROM provides early, minimal, stable identity.

Management objects provide early, minimal, independent health information.

Discovery records provide rich, versioned system description.

Capabilities provide authority.

These must remain separate.

<!-- AEMS-MIGRATED-SOURCE: docs/core-doctrines.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/core-doctrines.md -->
### Integrated source: `docs/core-doctrines.md`

# ATARIX Core Doctrines

Status: Baseline

## Purpose

This document records the non-negotiable architectural doctrines that guide ATARIX design, implementation, simulation, testing, and hardware validation.

These doctrines are intentionally short. They are review rules, not implementation details.

## Doctrines

### 1. No Direct Hardware Access

Software components do not directly access protected hardware, registers, buses, slots, or physical addresses.

All protected access occurs through services, capabilities, resources, and implementation layers.

### 2. Discovery Grants Visibility

Discovery reveals that resources or services exist.

Discovery does not grant authority to use them.

### 3. Capabilities Grant Authority

Capabilities are the authorization mechanism for protected operations.

A request without a valid capability is denied.

### 4. Deny By Default

The default answer to every protected operation is DENY.

Access is allowed only after all applicable validation, trust, ring, ownership, delegation, revocation, expiration, resource, and operation checks pass.

### 5. Hardware Is An Implementation Detail

Software targets services, not hardware placement.

A service may be implemented in FPGA logic, a CPU card, a peripheral card, a simulator, or a future node without changing the public contract.

### 6. Failures Must Be Observable

Failures must be detected, classified, reported, and traceable.

Silent failure is not acceptable.

### 7. Transport Independence

Services must not depend on a specific transport unless the transport itself is the service contract.

Mailbox, local calls, simulated links, FPGA paths, and future inter-node transports should preserve the same semantics.

### 8. Revocation Must Be Immediate

A revoked capability ceases to authorize operations immediately.

Revocation overrides ownership, trust, and convenience.

### 9. Least Privilege

Capabilities and services grant only the authority required for the requested function.

Delegation may reduce authority only.

### 10. Security Before Convenience

Architectural security takes precedence over shortcuts, performance hacks, simulator convenience, or direct-access workarounds.

### 11. No Address-Based Authority

Knowledge of an address, slot, register, mailbox endpoint, handle, service name, or transport identifier grants zero authority.

Authority is conveyed only by valid capabilities evaluated by policy.

### 12. Simulator Equals Hardware

The simulator is a reference implementation, not a shortcut or mock that bypasses security.

Simulator security rules, capability checks, mailbox validation, and service boundaries must match the future hardware model.

### 13. Ring Security Is Mandatory

ATARIX uses ring-based security boundaries.

Higher-trust rings may supervise, delegate, constrain, or revoke authority from lower-trust rings.

Lower-trust rings may not directly command, inspect, mutate, bypass, or impersonate higher-trust rings.

Ring boundaries are enforced by Supervisor policy, capability checks, mailbox validation, service dispatch, and fabric routing rules.

Capabilities grant authority only within the maximum authority permitted by the requester's ring and the target's ring policy.

## Required Access Path

The permitted protected-access path is:

```text
Application
    -> Service
    -> Capability Policy
    -> Ring Policy
    -> Resource
    -> Implementation
```

The forbidden path is:

```text
Application
    -> Address
    -> Hardware
```

## Security Decision Chain

Protected operations must satisfy the full security chain:

```text
Deny by default
    -> Ring boundary check
    -> Capability validation
    -> Revocation check
    -> Mailbox validation
    -> Service policy
    -> Resource policy
    -> Authorized operation
```

Failure at any step denies the operation.

## Suggested Ring Model

The initial ATARIX ring model is:

```text
Ring 0: Supervisor / Root Authority
Ring 1: Fabric Management / Trusted System Services
Ring 2: Device Services / Card Firmware
Ring 3: Applications / User Workloads
Ring 4: Untrusted or Quarantined Nodes
```

The exact ring count and names may evolve by ADR, but the existence of ring boundaries is mandatory.

## Review Rule

Every significant ATARIX change should answer:

```text
Which doctrine does this preserve?
Which ADR justifies this behavior?
Which public API expresses the contract?
Which test or bench procedure enforces it?
```

If the answer is unclear, the design is not ready.

## Result

These doctrines define the ATARIX architectural baseline.

Future subsystems may extend the system, but they must not weaken these rules without an explicit ADR.

<!-- AEMS-MIGRATED-SOURCE: docs/cpu-card-architecture-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/cpu-card-architecture-v1.md -->
### Integrated source: `docs/cpu-card-architecture-v1.md`

# ATARIX CPU Card Architecture v1

## Status

Draft hardware architecture specification.

This document defines the first-generation ATARIX CPU card architecture. It is informed by the Vega816 CPU Shim, CPU Buffer, DMA Controller, and Vector Pull Rewrite KiCad projects, but adapts those ideas for the ATARIX workstation-style fabric architecture.

## Purpose

The CPU card provides the primary W65C816 execution environment while isolating the raw CPU bus from the system backplane.

The CPU card should be independently bring-up capable, diagnosable, and serviceable.

## Design Rule

The W65C816 local bus stays on the CPU card.

The backplane sees a structured ATARIX fabric interface, not every raw W65C816 signal.

The CPU card shall be able to self-boot into a recovery monitor, but when a valid ATARIX backplane BIOS is discovered and validated, the CPU card shall hand off system boot policy to the backplane BIOS.

## Architectural Position

```text
W65C816 CPU
    |
Local CPU Bus
    |
Local ROM / RAM / I/O decode
    |
CPU Shim and Buffer Logic
    |
CPU-Card Bridge FPGA or CPLD
    |
ATARIX Fabric Interface
    |
DIN41612 Backplane
```

## Design Goals

1. Preserve simple W65C816 bring-up.
2. Keep the CPU bus observable.
3. Avoid exposing the full raw CPU bus across the backplane.
4. Support Vega816-derived CPU shim and buffer concepts.
5. Support vector-pull rewrite or equivalent interrupt assist logic.
6. Support wait-state generation and CPU pausing through RDY.
7. Support local boot even when the wider fabric is unavailable.
8. Support fabric-mediated DMA, discovery, interrupts, and mailboxes.
9. Prepare for future secondary CPU cards.
10. Support backplane BIOS discovery and handoff.
11. Populate the W65C816 native address space with the maximum practical CPU-local RAM.

## Required Functional Blocks

### W65C816 CPU

Primary processor:

```text
WDC W65C816S
```

Required CPU signals to preserve locally:

```text
A0-A23
D0-D7 or D0-D15 depending on board strategy
RWB
PHI2
RDY
IRQB
NMIB
ABORTB
RESB
VDA
VPA
VPB
MLB
BE
E
MX
```

The CPU card should expose these signals for diagnostics, but they are not all backplane signals.

### Local ROM / Bootstrap ROM

The CPU card should contain local boot ROM or a reliable ROM mapping path.

ROM responsibilities:

- Reset vector.
- Minimal CPU-card initialization.
- ROM monitor.
- Serial/network loader support.
- Recovery diagnostics.
- Basic memory and fabric tests.
- Backplane BIOS discovery.
- Backplane BIOS validation.
- Backplane BIOS handoff.

The CPU-card ROM is not the final system BIOS when a valid backplane BIOS is present.

Instead:

```text
CPU-Card ROM
    Local bootstrap and recovery authority.

Backplane BIOS
    System boot and platform policy authority after discovery and validation.
```

ROM may later become selectable between:

```text
Normal ROM
Golden ROM
Network-loaded image
Supervisor-selected image
```

### Backplane BIOS Handoff

At boot, the CPU-card ROM should determine whether it is running as:

```text
Standalone CPU-card recovery environment
```

or:

```text
CPU card installed in an ATARIX backplane
```

If a valid backplane BIOS is discovered, the CPU-card ROM should subordinate system boot policy to it.

Handoff requirements:

```text
Backplane BIOS identity verified
Backplane BIOS checksum or signature validated where practical
Supervisor state queried
Fabric identity queried
Memory baseline established
Recovery override checked
```

If validation fails, the CPU-card ROM must remain capable of entering recovery mode.

This design allows a CPU card to be self-testable outside the system while still allowing the backplane to own platform policy inside a complete ATARIX machine.

### Local RAM

The CPU card should provide the maximum practical CPU-local RAM directly addressable by the W65C816.

Target:

```text
16 MiB CPU-local addressable RAM
```

Rationale:

```text
The W65C816 exposes a 24-bit native address space.
The CPU card should not artificially constrain that address space below 16 MiB where practical.
```

The CPU card should still preserve deterministic regions for:

- Stack.
- Direct page.
- Monitor work area.
- Upload buffer.
- Netboot staging.
- Diagnostics.
- Recovery operation.

The first CPU card should be useful even if no external memory card exists.

Implementation options may include:

```text
SRAM
Pseudo-SRAM
SDRAM behind bridge logic
Mixed fast SRAM plus larger bridge-managed RAM
```

The architecture requires the addressability target, not a specific memory technology.

### Bank Zero Isolation

Each CPU card should own its own bank-zero environment.

Bank zero includes:

```text
$000000-$00FFFF
```

This preserves:

- Direct page behavior.
- Stack behavior.
- Interrupt-vector assumptions.
- ROM monitor assumptions.
- Per-CPU local execution state.

Future multi-CPU systems must not require every CPU to share the same physical bank zero.

### CPU Shim / Bridge Logic

The CPU card should include shim or bridge logic inspired by Vega816.

Responsibilities:

- CPU bus observation.
- Address decoding assistance.
- Wait-state insertion.
- RDY control.
- Local versus fabric access decision.
- Vector Pull Rewrite support if implemented.
- Fabric request generation.
- Debug-state reporting.
- Backplane BIOS aperture support.

This may be implemented with:

```text
Discrete logic for Rev A simplicity
CPLD
Small FPGA
```

The preferred long-term implementation is FPGA or CPLD-based.

### CPU Buffering

The card should buffer CPU bus signals where useful.

Goals:

- Improve signal integrity.
- Permit debug observation.
- Isolate CPU timing from downstream logic.
- Support controlled DMA or bridge access.

The Vega816 CPU Buffer project is the reference inspiration for this block.

### Vector Pull Rewrite

The CPU card should reserve support for Vector Pull Rewrite or an equivalent interrupt-assist mechanism.

Purpose:

- Detect W65C816 vector fetches.
- Support expanded interrupt source handling.
- Permit fabric interrupt-controller integration.
- Reduce interrupt dispatch ambiguity.

This should be documented further in:

```text
docs/vector-pull-rewrite-v1.md
```

### Fabric Bridge

The CPU-card bridge translates CPU activity into ATARIX fabric transactions.

Supported transaction types:

```text
Register Read
Register Write
Mailbox Send
Mailbox Receive
Discovery Read
Interrupt Acknowledge
DMA Request Submission
Fault Report Read
Backplane BIOS Read
```

The bridge should be able to stall the CPU with RDY while a fabric transaction completes.

## Local Address Map

The CPU card should implement a local map compatible with the Rev A memory model while targeting the full W65C816 native address space.

Candidate Rev A local map:

```text
$000000-$0000FF  Direct page
$000100-$0001FF  Stack
$000200-$00BFFF  Local RAM / monitor workspace
$00C000-$00DFFF  Local and fabric I/O aperture
$00E000-$00FFFF  ROM / vectors
$010000-$EFFFFF  CPU-local RAM
$F00000-$F7FFFF  Fabric / service windows, configurable
$F80000-$FBFFFF  Backplane BIOS aperture, configurable
$FC0000-$FFFFFF  Recovery ROM / vectors / high monitor region, configurable
```

Exact boundaries remain subject to memory-map specification and hardware decode constraints.

Fabric windows and backplane BIOS windows may be remapped or bank-selected by the bridge logic.

See:

```text
docs/memory-map.md
docs/address-space-architecture.md
docs/backplane-bios-v1.md
```

## Fabric Interface

The CPU card should expose a fabric-facing interface rather than a raw CPU bus.

Required logical groups:

```text
Power
Ground
Clock Reference
Reset Control
Management Plane
Control Plane
Event Plane
Data Plane
Discovery Interface
Backplane BIOS Interface
Diagnostic Signals
```

The exact connector pinout is deferred to:

```text
docs/backplane-pinout-v1.md
```

## Network Support

For Rev A, a network service path is mandatory because netboot and NTP are baseline capabilities.

Acceptable Rev A implementation:

```text
W5500 Ethernet Controller on CPU card
```

Alternative Rev A implementation:

```text
Dedicated boot-critical network service card
```

Requirement:

```text
The base Rev A system must not depend on a missing optional network card in order to perform netboot.
```

Recommended early network controller:

```text
W5500 Ethernet Controller
```

Reason:

- Simplifies TCP/IP burden.
- Supports netboot.
- Supports NTP.
- Supports remote diagnostics.
- Reduces dependency on a separate service card during bring-up.

Later revisions may move networking to a service card, but Rev A must provide a concrete network service path.

See:

```text
docs/netboot-ntp-v1.md
```

## Diagnostic Requirements

The CPU card must be highly observable.

Required diagnostic access:

```text
CPU Clock
CPU Reset
RDY
IRQB
NMIB
ABORTB
RWB
VDA
VPA
VPB
MLB
ROM Chip Select
RAM Chip Select
I/O Chip Select
Fabric Access Active
Backplane BIOS Access Active
Vector Pull Rewrite Active
Bank Zero Select
```

Recommended diagnostic access:

```text
Buffered address bus sample header
Buffered data bus sample header
Logic analyzer control-signal header
Power rail test points
Ground probe loops
CPU running LED
Wait-state LED
IRQ LED
Fault LED
Backplane BIOS handoff LED or status bit
```

See:

```text
docs/diagnostic-access-v1.md
```

## Reset and Clocking

The CPU card accepts reset control from the supervisor or backplane reset tree.

Required reset modes:

```text
Power-On Reset
Supervisor Reset
Manual Reset
Slot Reset
Recovery Hold-In-Reset
Software-Requested Reset
```

Clock modes:

```text
1 MHz Debug
4 MHz Bring-up
8 MHz Normal
14 MHz Experimental
Single-Step or Gated Debug Clock
```

See:

```text
docs/clock-reset-spec.md
```

## Interrupt Model

The CPU card receives normal IRQ/NMI/ABORT signaling from local logic and the fabric interrupt controller.

Expected behavior:

- IRQB handles normal maskable events.
- NMIB is reserved for severe or recovery-class events.
- ABORTB is reserved for future protection and bus-fault experiments.
- Vector Pull Rewrite may assist dispatch.

See:

```text
docs/interrupt-architecture-v1.md
```

## DMA Model

The CPU card does not grant unrestricted raw bus-master access to external cards.

DMA should be mediated by the fabric controller.

CPU-card-local DMA concepts from Vega816 may be useful, but long-term ATARIX DMA must follow the capability model.

See:

```text
docs/dma-engine-v1.md
docs/capability-model.md
```

## Discovery Record

The CPU card should expose a discovery record describing:

```text
Vendor ID
Device ID
CPU type
Card revision
ROM version
Local RAM size
Supported clock modes
Supported interrupt features
Supported fabric features
Backplane BIOS handoff support
Diagnostic capability flags
```

See:

```text
docs/discovery-rom-format.md
```

## Management Plane

The CPU card should include a low-speed management interface to the supervisor card.

Candidate functions:

- CPU heartbeat.
- Reset reason reporting.
- Temperature or voltage reporting if sensors exist.
- ROM version readout.
- Fault-state reporting.
- Hold-in-reset support.
- Backplane BIOS handoff status.

This management interface must remain useful even when the main CPU is not booting.

See:

```text
docs/card-identity-eeprom.md
```

## Relationship to Vega816

Keep:

- CPU shim idea.
- CPU buffer idea.
- RDY-based wait-state and pause control.
- Vector Pull Rewrite idea.
- Bank-zero awareness.
- Explicit DMA request/grant thinking.

Modify:

- BB816 connector assumptions.
- Raw shared-bus multiprocessing.
- Direct DMA-channel wiring.
- Interrupt routing details.

Avoid:

- Exposing the full CPU bus as the long-term system bus.

See:

```text
docs/vega816-analysis.md
```

## Initial Rev A CPU Card Recommendation

The first ATARIX CPU card should contain:

```text
W65C816S
16 MiB target CPU-local addressable RAM
Local bootstrap / recovery ROM or flash
CPU-local bus buffer/shim logic
Small FPGA or CPLD bridge
W5500 Ethernet or equivalent boot-critical network path
UART console path
Diagnostic headers and test points
Card identity EEPROM and management sensor cluster
DIN41612 backplane connector
```

## Open Questions

- Exact local RAM technology and topology.
- Exact ROM or flash part.
- Exact backplane BIOS aperture and validation scheme.
- Whether the first bridge is CPLD, small FPGA, or discrete logic.
- Whether W5500 lives on Rev A CPU card or a boot-critical service card.
- Exact management-plane electrical interface.
- Exact DIN41612 pin usage.
- Whether Vector Pull Rewrite is mandatory in Rev A or reserved for Rev B.
- Whether the CPU card includes its own oscillator or derives clock from the backplane.
- Whether local RAM is always bank zero or can be remapped by the bridge.

## Design Principle

The CPU card is not the whole computer.

It is a deterministic W65C816 execution node attached to a workstation-style ATARIX fabric.

It can recover itself locally, but in a complete system it should recognize and hand off platform boot policy to the backplane BIOS.

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-002-Architectural-Invariants.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-002-Architectural-Invariants.md -->
### Integrated source: `docs/design/ATX-DESIGN-002-Architectural-Invariants.md`

# ATX-DESIGN-002: Architectural Invariants

## Purpose

This document records architectural properties that must remain true unless explicitly changed by formal architecture review.

## Initial Invariants

- Object identity is never authority.
- Authority derives from capabilities and policy.
- Ambient authority is prohibited.
- Compatibility personalities must not weaken native security semantics.
- Inter-object communication occurs through defined transport mechanisms.
- State transitions must be auditable.
- Hardware acceleration may speed checks but must not bypass checks.
- Engineering changes require specification, review, verification, and gate evidence.

## Relationship To ATX-100

Stable invariants should be integrated into ATX-100 as canonical architectural constraints.

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-006-Performance-Philosophy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-006-Performance-Philosophy.md -->
### Integrated source: `docs/design/ATX-DESIGN-006-Performance-Philosophy.md`

# ATX-DESIGN-006: Performance Philosophy

## Purpose

This document defines how Atarix evaluates and applies performance optimizations.

## Principle

Performance is valuable only when it preserves architecture semantics.

Optimizations may reduce latency, memory use, copy cost, or CPU overhead. They must not weaken authority, policy, audit, recovery, human readability, or compatibility behavior.

## Initial Rules

- Acceleration is not authority.
- Cache state is not authority.
- Reachability is not permission.
- Fast paths must preserve slow-path semantics.
- Benchmarks should include correctness and auditability evidence.

<!-- AEMS-MIGRATED-SOURCE: docs/design/ATX-DESIGN-007-Security-Philosophy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/design/ATX-DESIGN-007-Security-Philosophy.md -->
### Integrated source: `docs/design/ATX-DESIGN-007-Security-Philosophy.md`

# ATX-DESIGN-007: Security Philosophy

## Purpose

This document defines the security philosophy behind Atarix.

## Principle

Atarix is deny-by-default and capability-oriented.

Names, handles, addresses, descriptors, cache entries, and lookup results may identify or accelerate access. They do not grant authority.

## Initial Security Rules

- Authority must be explicit.
- Capability checks must precede protected operations.
- Policy may further restrict capability-derived authority.
- Hardware may accelerate validation but may not bypass validation.
- Compatibility personalities must map foreign authority models into native Atarix authority rules.

<!-- AEMS-MIGRATED-SOURCE: docs/development-philosophy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/development-philosophy.md -->
### Integrated source: `docs/development-philosophy.md`

# ATARIX Development Philosophy

## Purpose

This document defines how ATARIX is developed.

Architecture decisions are influenced by four major traditions:

- Vega816 and BB816 practical 65C816 engineering
- Sun/NuBus/UPA workstation architecture
- curl development discipline
- OpenBSD security philosophy

## Documentation First

Architecture first.

Specification second.

Implementation third.

No major hardware, FPGA, firmware, or operating-system component should exist without a corresponding specification.

## Version Everything

Interfaces are versioned.

Examples:

- Discovery Format v1
- Mailbox Protocol v1
- DMA Descriptor v1
- Fabric Controller v1

Existing interfaces should not be silently changed.

## Small Changes

Prefer:

- Small commits
- Small pull requests
- Small reviews

Large architectural changes should be decomposed into reviewable units.

## Observable Systems

Every subsystem should expose:

- Status
- Counters
- Fault history
- Health metrics
- Recovery information

Debuggability is a first-class design goal.

## Security by Design

Borrowing from OpenBSD:

- Secure by default
- Least privilege
- Privilege separation
- Capability revocation
- Explicit trust boundaries
- Cryptography by reuse, not invention

## Cryptographic Standards

Preferred primitives:

- Ed25519 for signatures
- ChaCha20-Poly1305 for authenticated encryption
- BLAKE2b and SHA-256 for hashing

Avoid custom cryptography.

## Recovery First

Every subsystem should define:

- Failure modes
- Recovery behavior
- Reset behavior
- Diagnostic visibility

The machine should fail visibly and recover deliberately.

## Architecture Decision Records

Major decisions should be documented as ADRs.

Examples:

- Why ULX3S was selected
- Why RP2350 was selected
- Why the CPU bus remains local
- Why capabilities are used
- Why the supervisor is outside the ring model

## Ring and Capability Model

ATARIX uses:

- Rings for execution authority
- Capabilities for resource authority

Neither mechanism is sufficient alone.

## Design Principle

Prefer systems that are understandable, observable, auditable, recoverable, and secure over systems that are merely clever.

<!-- AEMS-MIGRATED-SOURCE: docs/dma-engine-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/dma-engine-v1.md -->
### Integrated source: `docs/dma-engine-v1.md`

# ATARIX DMA Engine v1

## Status

Draft specification for the ATARIX DMA subsystem.

This document defines DMA operation, descriptor formats, ownership rules, capability integration, fault handling, interrupt behavior, and the distinction between W65C816-native transfers and fabric/object-space transfers.

## Purpose

The DMA engine provides controlled movement of data between CPU-local memory, memory services, devices, FPGA services, storage services, and future accelerators.

Unlike traditional bus-mastering systems, DMA is not automatically trusted.

DMA operations require authorization.

## Data Model

DMA descriptors follow:

```text
docs/data-model-and-endianness.md
```

All multi-byte fields are little-endian.

The W65C816 uses u24 CPU-local addresses, but ATARIX DMA must also support u64 resource identifiers, u64 offsets, u64 object sizes, and future 32-bit / 64-bit CPU participants.

## Design Goals

1. Eliminate unrestricted DMA.
2. Permit future accelerators.
3. Support mailbox-driven operation.
4. Support capability validation.
5. Remain practical for a W65C816-based system.
6. Permit future FPGA enforcement.
7. Avoid treating u24 CPU-local addresses as universal system addresses.
8. Support large memory-service and storage-service objects without changing the core security model.

## DMA Architecture

The DMA engine is initially implemented as an FPGA fabric service.

The engine performs transfers on behalf of authorized requestors.

Requestors may include:

```text
W65C816 CPU Card
Future 32-bit CPU Card
Future 64-bit CPU Card
Supervisor
Network Service
Storage Service
Memory Service
Accelerator
Future Devices
```

## DMA Channels

Initial implementation:

```text
4 logical channels
```

Future revisions may increase channel count.

## Transfer Types

Supported transfers:

```text
CPU-Local Memory -> CPU-Local Memory
CPU-Local Memory -> Device
Device -> CPU-Local Memory
Memory Object -> CPU-Local Window
CPU-Local Window -> Memory Object
Memory Object -> Device
Device -> Memory Object
Memory Object -> Accelerator
Accelerator -> Memory Object
Memory Object -> Memory Object
```

Unsupported transfers must return an error.

## Descriptor Classes

ATARIX defines two descriptor classes:

```text
Native Descriptor
    Compact descriptor for W65C816-native u24 CPU-local transfers.

Fabric Descriptor
    Extended descriptor for fabric resources, memory objects, storage extents, accelerators, and future CPU cards.
```

Native descriptors are useful for early bring-up and local CPU-card transfers.

Fabric descriptors are required for large memory, storage, accelerator, and future multi-processor transfers.

## Native Descriptor Format

Version 1 native descriptor:

```text
Offset  Size  Field
+00     1     Descriptor Type = $01
+01     1     Flags
+02     1     Channel
+03     1     Priority
+04     3     Source CPU Address, u24
+07     3     Destination CPU Address, u24
+0A     4     Length, u32
+0E     2     Reserved
```

Descriptor size:

```text
16 bytes
```

Native addresses are W65C816 CPU-local addresses.

Native descriptor addresses are not universal ATARIX fabric addresses.

## Fabric Descriptor Format

Version 1 fabric descriptor:

```text
Offset  Size  Field
+00     1     Descriptor Type = $02
+01     1     Flags
+02     1     Channel
+03     1     Priority
+04     8     Source Resource ID, u64
+0C     8     Source Offset, u64
+14     8     Destination Resource ID, u64
+1C     8     Destination Offset, u64
+24     4     Length, u32 or extended-length escape
+28     8     Capability Reference, u64
+30     8     Transaction ID, u64
```

Descriptor size:

```text
56 bytes
```

The source and destination resource IDs may refer to:

- CPU-local memory apertures.
- Memory-service objects.
- Storage-service extents.
- Device buffers.
- Accelerator buffers.
- Fabric-managed queues.

## Extended Length Rule

The fabric descriptor length field normally uses u32.

```text
Length != $FFFFFFFF
    Length is encoded directly as u32.

Length == $FFFFFFFF
    Actual length is supplied by an associated extended descriptor or descriptor-chain record using u64.
```

Formats expected to represent persistent large objects should use u64 sizes in the owning object metadata.

DMA transfer lengths may remain u32 for common-case transfers, because very large transfers should usually be split into bounded operations for scheduling, watchdog, fault isolation, and recovery.

## Descriptor Flags

```text
Bit 0 Generate Completion Interrupt
Bit 1 Verify Capability
Bit 2 Source Is CPU-Local Address
Bit 3 Destination Is CPU-Local Address
Bit 4 Uses Extended Length
Bit 5 Scatter/Gather Descriptor
Bit 6 Reserved
Bit 7 Reserved
```

## Capability Validation

DMA operations require:

1. Valid DMA capability.
2. Valid source-resource capability.
3. Valid destination-resource capability.
4. Matching ownership chain.
5. Ring/security policy allowing the request.

Failure of any validation step causes immediate rejection.

Capability references are u64.

## DMA Lifecycle

### Step 1

Requestor creates a native or fabric descriptor.

### Step 2

Descriptor is submitted through mailbox service or a descriptor queue.

### Step 3

DMA engine validates descriptor format.

### Step 4

DMA engine validates capabilities.

### Step 5

DMA engine validates source and destination ranges.

### Step 6

Transfer executes.

### Step 7

Completion status is recorded.

### Step 8

Interrupt or mailbox completion is generated if requested.

## Register Interface

See:

```text
docs/register-map-v1.md
```

DMA block base:

```text
$00D300
```

Minimal native-register interface:

```text
+00 Control
+01 Status
+02 Source Address Low
+03 Source Address Mid
+04 Source Address High
+05 Destination Address Low
+06 Destination Address Mid
+07 Destination Address High
+08 Length Byte 0
+09 Length Byte 1
+0A Length Byte 2
+0B Length Byte 3
```

Future revisions may use descriptor queues instead of direct registers.

Register blocks exposing 32-bit or 64-bit fields must define snapshot and commit semantics in `docs/register-map-v1.md`.

## Status Codes

```text
$00 Idle
$01 Pending
$02 Active
$03 Complete
$04 Invalid Descriptor
$05 Invalid Capability
$06 Invalid Address
$07 Access Denied
$08 Timeout
$09 Internal Error
$0A Unsupported Descriptor Type
$0B Invalid Resource ID
$0C Range Exceeds Resource
```

## Interrupt Behavior

DMA completion may generate:

```text
DMA Complete
DMA Fault
DMA Timeout
```

Interrupt routing is controlled through the fabric interrupt controller.

## Ownership Rules

A requestor may only submit DMA work for resources it controls.

Examples:

Allowed:

```text
Network Service -> Network Buffer
Memory Service -> Granted CPU Window
Accelerator -> Granted Result Buffer
```

Denied:

```text
Network Service -> Arbitrary Kernel Memory
Accelerator -> CPU-local Bank Zero
Unknown Device -> Memory Service Object
```

## Accelerator Integration

Accelerators never receive unrestricted DMA authority.

Instead:

1. Memory capability issued.
2. DMA capability issued.
3. Job executed.
4. Results returned.
5. Capabilities revoked.

## Fault Reporting

DMA faults should report:

```text
Requestor
Descriptor Type
Capability Reference, u64
Source Resource ID or CPU Address
Destination Resource ID or CPU Address
Length
Transaction ID, if present
Fault Code
Timestamp, if available
```

Fault reports should be accessible through mailbox services and supervisor diagnostics.

## Future Enhancements

Potential future features:

```text
Scatter/Gather
Descriptor Queues
Priority Scheduling
Rate Limiting
Capability Caching
FPGA Enforcement Tables
u64 Extended-Length Descriptor Records
IOMMU-like Resource Windows
```

## Design Rule

DMA exists to move data.

DMA does not confer authority.

Authority is provided only through capabilities.

Native u24 DMA is a compatibility and bring-up path, not the full ATARIX memory model.

## Open Questions

- Final descriptor queue design.
- Maximum single transfer size.
- Channel scheduling algorithm.
- Capability lookup mechanism.
- Hardware enforcement strategy.
- Accelerator-specific DMA extensions.
- Whether extended-length transfers should be required to split at watchdog-safe boundaries.
- Whether resource IDs are assigned by fabric, supervisor, or capability broker.

<!-- AEMS-MIGRATED-SOURCE: docs/doctrine/ARC-REVIEW.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/doctrine/ARC-REVIEW.md -->
### Integrated source: `docs/doctrine/ARC-REVIEW.md`

# Atarix Architecture Review Doctrine

## Purpose

This document defines the review process used to evaluate Atarix architecture and implementation changes.

Architecture review exists to prevent implementation details from becoming accidental doctrine.

## Required Review Sections

Every architecture proposal and implementation pull request shall include:

```text
Affected Pillars:
Modified Pillars:
Relevant ARC Checks:
Security Doctrine Compliance:
Lifecycle and Cleanup Impact:
Authority Impact:
Persistence Impact:
Required Tests:
Known Exceptions:
```

## Pre-Code Review

Before implementation begins, answer:

1. Does this feature create authority?
2. Does this feature transfer authority?
3. Does this feature persist authority?
4. Does this feature expose authority?
5. Does this feature create new attack surface?
6. Does this feature create objects?
7. Does this feature create garbage?
8. Does this feature survive reboot?
9. Does this feature survive uninstall?
10. Does this feature comply with ARC-SEC?

If any answer is unknown, the specification must be written or updated before code is written.

## Human Comprehensibility

A system administrator should be able to answer:

- What is this object?
- Who owns it?
- Why does it exist?
- What authority does it possess?
- How do I remove it?

Human-readable namespace and object observability are security features.

## Implementation Detail Rule

Implementation details shall not be exposed as public contracts.

Examples include table slots, memory addresses, mailbox numbers, internal indexes, CPU-local pointers, physical node addresses, and raw device locations.

<!-- AEMS-MIGRATED-SOURCE: docs/doctrine/ARC-SEC.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/doctrine/ARC-SEC.md -->
### Integrated source: `docs/doctrine/ARC-SEC.md`

# Atarix Security Doctrine

## Status

Normative draft v0.1

## Purpose

This document defines the core security doctrine for Atarix. All specifications, subsystems, services, protocols, runtimes, and applications shall be reviewed against this doctrine.

The purpose of this doctrine is not merely to prevent compromise. The purpose is to ensure that compromise does not automatically become authority.

## Core Security Philosophy

Atarix assumes:

- Software defects exist.
- Remote code execution will occur.
- Services may be compromised.
- User identities may be stolen.
- Internal nodes are not inherently trusted.
- Networks are hostile.
- Cleanup failures become security failures.

Therefore, Atarix protects authority, not merely code.

## ARC-SEC Rules

1. Authority must never be inferred from execution.
2. Connectivity does not imply trust.
3. Compromise is expected and must be containable.
4. No ambient authority.
5. Explicit lifecycle.
6. No orphans.
7. Leases by default.
8. Crash recovery is architectural.
9. Persistence must be requested.
10. Authority cleanup.
11. Garbage must be observable.
12. The system is not exempt.
13. Minimize attack surface.
14. No privilege escalation by identity.
15. Identity theft is not authority theft.
16. Secure by default.
17. Fail closed.
18. Verifiable state.
19. Reproducible recovery.
20. Least persistence.

## Summary

- Authority is explicit.
- Trust is explicit.
- Ownership is explicit.
- Persistence is explicit.
- Cleanup is explicit.
- System objects are not exempt.
- Identity is not authority.
- Connectivity is not trust.
- Compromise is expected.
- Containment is mandatory.

<!-- AEMS-MIGRATED-SOURCE: docs/engineering/Engineering-Gate.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/engineering/Engineering-Gate.md -->
### Integrated source: `docs/engineering/Engineering-Gate.md`

# Engineering Gate

## Status

Draft v0.2

## Purpose

The Atarix Engineering Gate is the single validation entry point for repository health.

Local command:

```sh
bash scripts/engineering/run_gate.sh
```

GitHub Actions workflow:

```text
.github/workflows/engineering-gate.yml
```

## Current Gate Phases

```text
1. Environment report
2. Repository structure checks
3. RTL file inventory checks
4. Architecture text invariant checks
5. Public header self-compile in C
6. Public header self-compile in C++
7. Warning-as-error enforcement for public headers
8. Verilator RTL lint
9. ATX-SPEC-020 RTL simulation gate
10. Artifact and summary generation
```

## Required Invariants

The gate currently verifies these architecture invariants:

```text
Lookup acceleration is not authority.
Every accelerated lookup must be reconstructable inside a bounded audit window.
POSIX is a compatibility personality.
```

## Current Build Tools

```text
gcc
g++
iverilog
vvp
verilator
```

## Current Gate Scripts

```text
scripts/engineering/run_gate.sh
scripts/engineering/check_headers.sh
scripts/engineering/check_rtl_lint.sh
scripts/rtl/run_atx_spec_020_sims.sh
```

## Current Artifacts

```text
build/engineering-gate/artifacts/summary.txt
build/engineering-gate/artifacts/header-checks.log
build/engineering-gate/artifacts/verilator-lint.log
build/engineering-gate/artifacts/atx_spec_020_rtl/
```

The nested RTL artifact directory may include:

```text
summary.txt
atx_spec_020_accelerator_tb.log
atx_spec_020_modules_tb.log
atx_spec_020_accelerator_tb.vcd
atx_spec_020_modules_tb.vcd
```

## Header Gate

Every public header under:

```text
include/atarix/*.h
```

is compiled independently in C and C++ mode.

The intent is to catch:

```text
missing includes
include-order dependency bugs
C/C++ ABI wrapper mistakes
warnings in public headers
undeclared public types
```

## RTL Gate

The RTL gate currently runs:

```text
Verilator lint
Icarus Verilog accelerator simulation
Icarus Verilog module validation simulation
```

The RTL simulation validates:

```text
authorization gates
audit-before-response behavior
SIMD-style control-byte probe path
scalar baseline probe path
Krapivin stepper math
Elias-Fano bounded decode
audit log window readback
```

## Extension Plan

Future gate phases should add:

```text
Markdown link validation
Architecture review reference validation
C source compilation
Unit tests
ECP5 synthesis smoke test
Timing report generation
Security invariant tests
Policy invariant tests
Recovery invariant tests
Performance regression thresholds
Randomized RTL tests
Coverage reporting
```

## Rule

The gate should become stricter over time, but it should remain one command.

```text
One project.
One gate.
One answer.
```

<!-- AEMS-MIGRATED-SOURCE: docs/hardware/bench-validation-plan-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/hardware/bench-validation-plan-v1.md -->
### Integrated source: `docs/hardware/bench-validation-plan-v1.md`

# Hardware Bench Validation Plan v1

Status: Draft

## Purpose

This plan defines the first ATARIX physical validation sequence.

The goal is not to boot the W65C816 first.

The goal is to prove the Supervisor-to-Fabric architecture on the bench before expensive CPU-card integration begins.

Primary target:

```text
RP2350 Supervisor
        <->
ECP5 Fabric
```

## Architectural Requirements

Bench validation must preserve the ATARIX doctrines:

```text
Supervisor owns trust.
Fabric enforces trust.
Everything is a service.
Discovery advertises.
Directory describes.
Capabilities authorize.
No direct hardware access.
Addresses are implementation details.
Deny by default.
Knowledge is not authority.
```

A hardware test is not complete merely because communication works.

It must also avoid creating bypass paths around the service and capability model.

## Phase 0: Bench Infrastructure

Goal:

```text
Establish reliable programming, power, measurement, and recovery tools.
```

Verify:

```text
ULX3S powers reliably.
RP2350 powers reliably.
USB serial access works.
Programming chain works.
Logic analyzer captures expected signals.
Oscilloscope probes expected rails and clocks.
Bench supply current limits are set.
Grounding is known and stable.
```

Success criteria:

```text
Both devices can be reflashed repeatedly.
Power cycling is repeatable.
Serial console is available.
Logic analyzer capture is repeatable.
```

## Phase 1: Supervisor Bring-Up

Goal:

```text
RP2350 boots and owns basic machine-control responsibilities.
```

Tests:

```text
Heartbeat LED
Serial boot banner
Watchdog setup
Reset output assertion
Reset output release
Power-good input readback
Fault input readback
```

Measurements:

```text
Boot time
Reset pulse width
Watchdog timeout
Power-good debounce time
Heartbeat period
```

Success criteria:

```text
Supervisor can boot, report status, assert reset, release reset, and recover from watchdog timeout.
```

## Phase 2: Fabric Bring-Up

Goal:

```text
ECP5 configures and becomes visible to Supervisor.
```

Minimum Fabric register block:

```text
0x0000 FABRIC_ID
0x0004 FABRIC_VERSION
0x0008 BUILD_ID
0x000C STATUS
0x0010 SCRATCH0
0x0014 SCRATCH1
0x0018 HEARTBEAT
0x001C FAULT_STATUS
```

Tests:

```text
Configuration done indication
Version register read
Build register read
Scratch register write
Scratch register readback
Heartbeat register changes
Fault status read
```

Success criteria:

```text
Supervisor can read and write the minimal Fabric register block repeatedly without protocol failure.
```

## Phase 3: Supervisor-Fabric Mailbox Validation

Goal:

```text
Establish reliable packet transport between Supervisor and Fabric.
```

Candidate mailbox header:

```c
struct atarix_mailbox_header_v1 {
    uint16_t type;
    uint16_t length;
    uint32_t sequence;
    uint32_t crc32;
};
```

Initial message types:

```text
PING
PONG
READ_REGISTER
WRITE_REGISTER
STATUS_QUERY
STATUS_REPLY
RESET_REQUEST
FAULT_REPORT
DISCOVERY_RECORD_PUSH
CAPABILITY_EVALUATE
```

Tests:

```text
Send packet
Receive packet
Sequence increment
CRC validation
Timeout handling
Retry handling
Duplicate packet handling
Out-of-order packet handling
Oversized packet rejection
Malformed length rejection
```

Fault injection:

```text
Drop packet
Duplicate packet
Corrupt CRC
Corrupt length
Corrupt sequence
Reset Fabric mid-transaction
Reset Supervisor mid-transaction
Hold Fabric busy
Send unknown message type
```

Success criteria:

```text
100,000 packets exchanged with zero unhandled protocol failures.
All injected faults are detected and reported.
No malformed packet grants authority or causes unsafe hardware action.
```

## Phase 4: Discovery Hardware Validation

Goal:

```text
Transport and validate Discovery records over the Supervisor-Fabric link.
```

Synthetic records:

```text
UART Service
GPIO Service
RTC Service
Fabric Status Service
Supervisor Health Service
```

Tests:

```text
BEGIN record accepted
SERVICE record accepted
END record accepted
CRC checked
Enumeration works
Iteration works
Lookup by handle works
Duplicate handles rejected
Missing END rejected
Invalid length rejected
Unknown record handled safely
```

Security requirement:

```text
Discovery reveals existence only.
Discovery does not grant authority.
```

Success criteria:

```text
Hardware transport can carry Discovery records equivalent to the structures covered by CI tests.
Discovery-only access cannot operate a protected resource.
```

## Phase 5: Capability Policy Hardware Validation

Goal:

```text
Exercise Capability Policy through the hardware transport before CPU-card integration.
```

Synthetic principals:

```text
Supervisor
Fabric
Kernel
Driver
Service
Application
```

Tests:

```text
Valid capability allowed
Invalid capability denied
Owner mismatch denied
Ring violation denied
Delegation violation denied
Revoked capability denied
Expired capability denied
Unknown handle denied
Unsupported operation denied
```

Security validation:

```text
Discovery only        -> DENY
Directory only        -> DENY
Known handle only     -> DENY
Known address only    -> DENY
Valid capability      -> EVALUATE
Invalid capability    -> DENY
Revoked capability    -> DENY
Expired capability    -> DENY
```

Success criteria:

```text
The bench proves that Discovery and Directory do not bypass Capability Policy.
Fabric enforcement remains mandatory for protected operations.
```

## Phase 6: First CPU Card Integration

Goal:

```text
Connect the W65C816 CPU card only after Supervisor, Fabric, mailbox, Discovery, and Capability paths are validated.
```

Preconditions:

```text
Supervisor validated
Fabric register block validated
Mailbox transport validated
Discovery transport validated
Capability policy validation complete
Fault injection complete
```

Initial CPU-card tests:

```text
Reset hold
Reset release
Clock present
ROM visible locally
SRAM visible locally
Vector fetch
Diagnostic heartbeat
Fabric service request
```

Success criteria:

```text
CPU card interacts with ATARIX through service paths rather than direct global hardware access.
```

## Bench Issues

Recommended issue sequence:

```text
Bench-001 RP2350 Bring-Up
Bench-002 ECP5 Bring-Up
Bench-003 Supervisor-Fabric Mailbox
Bench-004 Discovery Record Transport
Bench-005 Capability Policy Hardware Validation
Bench-006 First CPU Card Integration
```

## Required Instruments

Minimum bench setup:

```text
Bench power supply with current limiting
USB serial access
Logic analyzer
Oscilloscope
Known-good USB cables
Known-good programming cables
ESD-safe work surface
Common ground reference
```

Recommended measurements:

```text
3.3 V rail stability
1.2 V FPGA core rail stability
Clock stability
Reset pulse width
Mailbox signal timing
SPI/UART/parallel timing depending on selected link
Current draw during configuration
Current draw during mailbox stress
```

## Exit Criteria

Hardware Validation Sprint 1 is complete when:

```text
RP2350 boots.
ECP5 configures.
Supervisor can reset Fabric.
Supervisor can read Fabric version.
Supervisor can write and read scratch registers.
Mailbox packet exchange works.
Fault injection is detected safely.
Discovery records can be transported.
Capability decisions can be exercised over the link.
Unauthorized access attempts are denied.
```

## Result

This plan turns the ATARIX architecture into measurable bench work.

The first physical milestone is reliable Supervisor-Fabric communication with security-preserving service abstraction.

<!-- AEMS-MIGRATED-SOURCE: docs/interrupt-architecture-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/interrupt-architecture-v1.md -->
### Integrated source: `docs/interrupt-architecture-v1.md`

# ATARIX Interrupt Architecture v1

## Status

Draft interrupt architecture specification.

This document defines interrupt sources, routing, priority, masking, acknowledgement, supervisor interaction, and future multi-CPU delivery for ATARIX.

## Purpose

Interrupts connect the W65C816 CPU, supervisor controller, FPGA fabric, mailbox engine, DMA engine, timers, network services, and future accelerators.

The interrupt architecture must remain simple enough for Rev A bring-up while allowing a fabric-mediated interrupt controller in later revisions.

## Design Goals

1. Keep Rev A interrupt handling simple and observable.
2. Permit Rev C FPGA-mediated interrupt routing.
3. Support interrupt masking and prioritization.
4. Support mailbox and DMA completion events.
5. Prepare for future multi-CPU routing.
6. Preserve a deterministic recovery path.

## W65C816 Interrupt Context

The W65C816 supports:

```text
IRQ
NMI
ABORT
RESET
```

For ATARIX:

- RESET is controlled by the supervisor and board reset logic.
- NMI is reserved for high-priority fault or recovery events.
- IRQ is used for normal maskable device and fabric events.
- ABORT is reserved for future bus fault or protection experiments.

## Interrupt Classes

```text
Class 0 Reset / Boot Control
Class 1 Non-Maskable Fault
Class 2 Fabric Fault
Class 3 Timer
Class 4 Mailbox
Class 5 DMA
Class 6 Network
Class 7 Storage
Class 8 Accelerator
Class 9 External Slot Device
Class 10 Debug / Monitor
```

## Initial Interrupt Sources

```text
0 UART
1 Supervisor
2 Timer
3 Mailbox
4 DMA
5 Network
6 Accelerator
7 External Slot
8 Fabric Fault
9 Debug Request
```

These source numbers extend the initial interrupt list in `docs/register-map-v1.md` and should be treated as the preferred v1 source list.

## Interrupt Controller Register Block

See:

```text
docs/register-map-v1.md
```

Base:

```text
$00D100
```

Minimum registers:

```text
+00 Pending Low
+01 Pending High
+02 Mask Low
+03 Mask High
+04 Priority Select
+05 Acknowledge
+06 Route Target
+07 Status
```

The original four-register draft is considered the minimal compatibility subset.

## Pending Register

Each bit corresponds to an interrupt source.

```text
Bit 0 UART
Bit 1 Supervisor
Bit 2 Timer
Bit 3 Mailbox
Bit 4 DMA
Bit 5 Network
Bit 6 Accelerator
Bit 7 External Slot
Bit 8 Fabric Fault
Bit 9 Debug Request
```

## Mask Register

A set bit masks the corresponding interrupt source.

Masking prevents delivery to CPU IRQ but does not necessarily clear the pending bit.

## Priority Model

Initial priority order:

```text
Highest: NMI / Supervisor Fault
Fabric Fault
DMA Fault
Mailbox Fault
Timer
UART
Network
Storage
Accelerator Completion
External Slot
Lowest: Debug Request
```

The priority model may later become programmable.

## Acknowledge Model

Interrupts should be acknowledged explicitly.

Acknowledge sequence:

1. CPU enters interrupt handler.
2. CPU reads pending register.
3. CPU services highest-priority unmasked source.
4. CPU writes source number or bit mask to acknowledge register.
5. Controller clears pending status if the source is no longer asserted.

Level-triggered sources may reassert until the device condition is cleared.

## IRQ Routing

Rev A:

```text
Device IRQ lines may be directly ORed or simply decoded.
```

Rev C and later:

```text
FPGA fabric routes device events to CPU IRQ.
```

The FPGA interrupt controller should expose pending, mask, priority, and acknowledgement state.

## NMI Use

NMI is reserved for conditions requiring immediate attention.

Candidate NMI sources:

- Supervisor emergency fault.
- Fatal fabric fault.
- Recovery button.
- Watchdog expiration.

NMI must not be used for routine device completion.

## ABORT Use

ABORT is reserved for future protection and bus-error experiments.

Possible future uses:

- Invalid memory window access.
- Capability violation.
- Bus timeout.
- Fabric protection fault.

ABORT should remain disabled or unused until the system can report faults reliably.

## Mailbox Interrupts

Mailbox interrupts indicate:

- Message received.
- Response available.
- Mailbox fault.

Mailbox handlers should read mailbox status before reading payload data.

See:

```text
docs/mailbox-protocol-v1.md
```

## DMA Interrupts

DMA interrupts indicate:

- Transfer complete.
- Transfer fault.
- Capability validation failure.
- Timeout.

See:

```text
docs/dma-engine-v1.md
```

## Timer Interrupts

Timer interrupts support:

- Monitor delays.
- Scheduler ticks.
- Watchdog service.
- Benchmarking.

Timer interrupts should be maskable.

## Supervisor Interrupts

Supervisor-generated interrupts may indicate:

- Reset reason available.
- Fault log update.
- Recovery request.
- Power or thermal warning.
- Configuration event.

Supervisor emergency events may use NMI depending on severity.

## Multi-CPU Routing

Future Rev D systems may route interrupts to specific CPU cards.

Routing targets:

```text
$00 Broadcast
$01 Primary CPU
$02 Secondary CPU
$03 Supervisor
$04 Fabric Service
```

Future routing policy may include:

- Fixed CPU ownership.
- Round-robin delivery.
- Priority CPU delivery.
- Supervisor-directed routing.

## Fault Reporting

Interrupt-related faults should report:

```text
Source
Class
Pending Mask
Delivery Target
Fault Code
Timestamp or Counter
```

## Boot Defaults

On reset:

```text
All maskable interrupts disabled
Pending state cleared if safe
NMI reserved for recovery/fault
DMA interrupts disabled until DMA initialized
Mailbox interrupts disabled until mailbox initialized
Timer interrupts disabled until monitor enables them
```

## Design Rule

Interrupts notify.

They do not transfer ownership, grant authority, or imply trust.

Authority remains controlled by the capability model.

## Open Questions

- Final number of interrupt sources.
- Whether priorities are fixed or programmable.
- Whether interrupt status should be 16-bit or 32-bit.
- Whether ABORT should be used for capability faults.
- Multi-CPU delivery policy.
- Exact relationship between fabric faults and NMI.

<!-- AEMS-MIGRATED-SOURCE: docs/mailbox-protocol-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/mailbox-protocol-v1.md -->
### Integrated source: `docs/mailbox-protocol-v1.md`

# ATARIX Mailbox Protocol v1

Status: Baseline

## Purpose

The ATARIX Mailbox Protocol defines the first transport contract between Supervisor, Fabric, simulator components, and future hardware implementations.

The mailbox is not an authority mechanism. It transports requests and responses. Authority remains with Capability Policy and Fabric enforcement.

## Header Format

Mailbox v1 uses a fixed 16-byte header:

```c
struct atarix_mailbox_header_v1 {
    uint32_t magic;
    uint16_t type;
    uint16_t length;
    uint32_t sequence;
    uint32_t crc32;
};
```

Current constants:

```text
Magic:              ATARIX_MAILBOX_MAGIC_V1
Header size:        16 bytes
Maximum payload:    4096 bytes
```

## Message Types

Mailbox v1 defines the following message types:

```text
INVALID
PING
PONG
VERSION_QUERY
VERSION_REPLY
STATUS_QUERY
STATUS_REPLY
READ_REGISTER
WRITE_REGISTER
RESET_REQUEST
FAULT_REPORT
DISCOVERY_RECORD_PUSH
CAPABILITY_EVALUATE
```

Unknown or invalid message types are rejected.

## Validation Order

Receivers validate mailbox traffic before dispatch.

Required validation order:

```text
Header present
Payload present when length is nonzero
Magic valid
Message type known
Length valid
Length matches payload buffer
CRC valid
Sequence accepted
Capability valid if required
Dispatch
```

Invalid traffic must not reach service dispatch.

## CRC Rules

The CRC covers the payload.

A CRC mismatch rejects the message.

CRC failure is observable and must return an explicit failure status.

## Length Rules

The header length must match the actual payload length.

Payload length must not exceed the protocol maximum.

Oversized payloads are rejected.

Malformed lengths are rejected.

## Sequence Rules

Mailbox sequences are tracked per receiver window.

Rules:

```text
First sequence accepted.
Exact duplicate rejected.
Older sequence rejected as out-of-order.
Sequence gaps rejected.
Next expected sequence accepted.
```

Sequence handling exists to detect replay, duplicate delivery, out-of-order delivery, and transport corruption.

## Dispatch Rules

Message dispatch occurs only after validation.

Preferred implementation for larger message sets is table-based dispatch:

```text
handler = dispatch_table[message_type]
```

Dispatch tables must not bypass validation.

## Security Rules

Mailbox routing does not grant authority.

The following are not authority:

```text
Valid mailbox header
Valid CRC
Known message type
Known sequence
Known endpoint
Known service handle
```

Capability Policy grants authority.

Protected operations must validate capabilities before resource access.

## Failure Rules

Failures must be observable.

Mailbox implementations must explicitly report:

```text
NULL input
Invalid header
Invalid length
CRC mismatch
Sequence error
Duplicate packet
Out-of-order packet
Unknown message type
Capability failure
```

Silent failure is not acceptable.

## Simulator Requirements

The simulator must follow the same mailbox rules as hardware.

Simulator implementations must reject malformed mailbox traffic in the same way future hardware-facing implementations are expected to reject it.

## Current Test Coverage

Mailbox v1 is covered by tests for:

```text
CRC validation
General validation
Sequence validation
Duplicate rejection
Out-of-order rejection
Unknown type rejection
Maximum payload handling
Oversized payload rejection
```

## Result

Mailbox Protocol v1 provides the first ATARIX transport contract.

It is intentionally small, deterministic, security-first, and suitable for simulation before hardware exists.

<!-- AEMS-MIGRATED-SOURCE: docs/netboot-ntp-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/netboot-ntp-v1.md -->
### Integrated source: `docs/netboot-ntp-v1.md`

# ATARIX Netboot and NTP Boot Services v1

## Status

Draft boot-services specification.

This document records the decision that ATARIX Rev A should support FPGA-assisted networking services early enough to enable netboot and automatic time synchronization at boot.

## Purpose

Because ATARIX includes an FPGA fabric controller from Rev A, networking should be treated as a core bring-up and recovery feature rather than a late operating-system feature.

Netboot allows firmware and kernel images to be loaded without repeatedly programming ROMs or local storage.

NTP allows the RTC and fault logs to be corrected automatically at boot when a network is available.

## Design Goals

1. Support network-assisted bring-up.
2. Support automatic time setting at boot.
3. Keep recovery possible without a network.
4. Keep the boot path observable through UART diagnostics.
5. Avoid making network success mandatory for local boot.
6. Preserve a safe fallback path using RTC, ROM monitor, and recovery mode.

## Hardware Assumptions

Early networking may be provided by:

```text
W5500 Ethernet controller
FPGA-assisted network interface
Service card network controller
```

The W5500 remains the preferred early candidate because it minimizes TCP/IP stack burden on the W65C816.

## Boot-Time Network Services

Required boot-time services:

```text
Link detection
MAC address readout
DHCP or static IP configuration
NTP time synchronization
TFTP or simple netboot image retrieval
Boot-status reporting over UART
```

Optional later services:

```text
BOOTP
PXE-like boot descriptors
HTTP boot
Remote monitor commands
Remote fault-log retrieval
```

## Boot Order Integration

Network services fit into the boot sequence after minimal hardware initialization and before kernel handoff.

Recommended order:

```text
Power stable
Supervisor start
FPGA fabric ready
CPU reset release
ROM monitor start
Basic diagnostics
Discovery scan
Network service discovery
RTC read
NTP sync attempt
Netboot attempt if configured
Fallback to local boot or monitor
```

See:

```text
docs/boot-sequence-v1.md
```

## Time Initialization Policy

Boot-time time source priority:

```text
1. Valid NTP time
2. Valid RTC time
3. Supervisor retained timestamp
4. Build-time firmware timestamp
5. Unknown time
```

NTP should update the RTC only after a sanity check.

Sanity checks should include:

```text
NTP response valid
Time newer than firmware build time
Time not wildly outside expected range
Network source accepted by configuration
```

## RTC Role

The RTC remains required even with NTP.

RTC responsibilities:

```text
Maintain time without network
Timestamp watchdog resets
Timestamp fault logs
Timestamp recovery-mode entry
Provide boot-time fallback time
```

NTP corrects the RTC when available; it does not replace it.

## Netboot Modes

Candidate netboot modes:

```text
Disabled
Manual Only
Try Netboot Then Local
Require Netboot
Recovery Netboot
```

These may be selected by:

```text
DIP switch
Supervisor configuration
ROM monitor command
Stored configuration EEPROM
```

## Netboot Image Flow

A minimal netboot flow:

```text
1. Initialize network device
2. Acquire IP configuration
3. Locate boot server
4. Request boot image
5. Load image into RAM
6. Verify checksum
7. Prepare handoff structure
8. Jump to loader or kernel
```

## Boot Image Requirements

A boot image should include:

```text
Magic number
Image type
Load address
Entry point
Image length
Checksum or hash
Version string
Optional capability requirements
```

## Failure Policy

Network failure must not brick the system.

If NTP fails:

```text
Use RTC if valid
Mark time as unsynchronized
Continue boot unless policy requires synchronized time
```

If netboot fails:

```text
Fallback to local boot if permitted
Enter monitor if no valid boot source exists
Record failure reason
```

## Security Policy

Rev A may begin with checksum-based validation.

Future revisions should support:

```text
Signed boot images
Trusted boot-server configuration
Capability-restricted boot services
Network-service isolation
```

Netboot must not automatically grant DMA, accelerator, or privileged fabric access.

## Diagnostic Output

ROM monitor boot logs should show:

```text
Network device detected
Link status
MAC address
IP address
NTP result
RTC status
Netboot mode
Boot server
Image name
Image verification result
Fallback reason if any
```

## FPGA Role

The Rev A FPGA fabric controller may assist with:

```text
Network device register windowing
Interrupt routing
DMA gating for network buffers
Boot status LEDs
Timeout enforcement
Fault reporting
```

## Design Rule

Network boot and time synchronization are convenience and reliability features.

They must improve bring-up and maintenance without becoming single points of failure.

## Open Questions

- DHCP versus static IP as the Rev A default.
- TFTP versus simpler custom loader protocol.
- Whether W5500 is on CPU card, service card, or backplane service area.
- Boot image header format.
- Whether signed images are required before Rev B.
- How NTP sanity thresholds are configured.
- Whether RTC is on the supervisor card or fabric-controller/backplane card.

<!-- AEMS-MIGRATED-SOURCE: docs/operation-id-registry-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/operation-id-registry-v1.md -->
### Integrated source: `docs/operation-id-registry-v1.md`

# Operation ID Registry v1

Status: Draft v1

## Purpose

This document defines the canonical ATARIX Operation ID registry.

Operation IDs identify actions that may be authorized by Capability Records.

Operation IDs are used by:

- Capability Records
- Supervisor policy
- Ring security policy
- Service Directory binding
- Validation tools
- Monitor commands

An Operation ID identifies an action.

An Operation ID does not grant authority.

## Architectural Rules

```text
Identity identifies.
Discovery describes.
Trust evaluates.
Capabilities authorize.
Rings constrain authority.
Directory locates.
Transport operates.
```

## Operation ID Ranges

```text
0x00000000              Reserved / invalid
0x00000001-0x00000FFF   Core operations
0x00001000-0x00001FFF   Instrumentation operations
0x00002000-0x00002FFF   CPU operations
0x00003000-0x00003FFF   Network operations
0x00004000-0x00004FFF   Storage operations
0x80000000-0xFFFFFFFF   Experimental / homebrew operations
```

## Core Operations

```text
0x00000001  OP_READ
0x00000002  OP_WRITE
0x00000003  OP_CONTROL
0x00000004  OP_RESET
0x00000005  OP_CAPTURE
0x00000006  OP_STIMULATE
0x00000007  OP_BOOT
0x00000008  OP_RECOVER
0x00000009  OP_PROVISION
0x0000000A  OP_ENUMERATE
0x0000000B  OP_VALIDATE
0x0000000C  OP_MONITOR
0x0000000D  OP_BIND
0x0000000E  OP_DELEGATE
0x0000000F  OP_REVOKE
```

## Instrumentation Operations

```text
0x00001000  OP_TRACE_START
0x00001001  OP_TRACE_STOP
0x00001002  OP_TRACE_EXPORT
0x00001003  OP_TRIGGER_ARM
0x00001004  OP_TRIGGER_FIRE
0x00001005  OP_PATTERN_LOAD
0x00001006  OP_PATTERN_START
0x00001007  OP_PATTERN_STOP
```

## CPU Operations

```text
0x00002000  OP_CPU_READ_HEALTH
0x00002001  OP_CPU_READ_BOOT_STATUS
0x00002002  OP_CPU_HOLD_RESET
0x00002003  OP_CPU_RELEASE_RESET
0x00002004  OP_CPU_CAPTURE_TRACE
0x00002005  OP_CPU_VALIDATE
```

## Security Notes

Operation IDs are interpreted through Capability Records and ring policy.

Examples:

```text
OP_READ
    may be safe for lower rings depending on target

OP_STIMULATE
    generally requires Ring -2 or Ring -1

OP_PROVISION
    generally requires Ring -2

OP_BOOT
    requires explicit boot authority
```

## Allocation Policy

Assigned IDs shall not be reused.

Deprecated IDs remain reserved permanently.

Operation IDs are allocated by registry update.

## Related Documents

- capability-record-format-v1.md
- ADR-RING-SECURITY-MODEL.md
- service-directory-format-v1.md

<!-- AEMS-MIGRATED-SOURCE: docs/resource-type-registry-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/resource-type-registry-v1.md -->
### Integrated source: `docs/resource-type-registry-v1.md`

# Resource Type Registry v1

Status: Draft v1

## Purpose

This document defines the canonical ATARIX Resource Type ID registry.

Resource Type IDs identify classes of resources exposed through the Enumeration Fabric.

Resource Type IDs are used by:

- Discovery ROM
- Enumeration services
- Capability records
- Directory services
- Validation services
- Instrumentation services

A Resource Type ID identifies a class of resource.

A Resource Type ID does not identify a specific instance.

Specific instances are identified through handles.

## Architectural Rules

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Rings constrain authority.
Directory locates.
Transport operates.
```

Resource Type IDs do not grant authority.

Resource Type IDs do not imply trust.

Resource Type IDs do not imply capability.

## Resource ID Ranges

```text
0x0000          Reserved / invalid
0x0001-0x0FFF   Core resources
0x1000-0x1FFF   Instrumentation resources
0x2000-0x2FFF   CPU resources
0x3000-0x3FFF   Network resources
0x4000-0x4FFF   Storage resources
0x5000-0x5FFF   Reserved
0x6000-0x6FFF   Accelerator resources
0x7000-0x7FFF   Reserved
0x8000-0xFFFF   Experimental / homebrew resources
```

## Core Resources

```text
0x0001  RESOURCE_MAILBOX
0x0002  RESOURCE_DMA
0x0003  RESOURCE_TIMER
0x0004  RESOURCE_INTERRUPT_CONTROLLER
0x0005  RESOURCE_GPIO
0x0006  RESOURCE_UART
0x0007  RESOURCE_FABRIC_MEMORY
0x0008  RESOURCE_SHARED_MEMORY
0x0009  RESOURCE_EVENT_QUEUE

0x000A  RESOURCE_ENUMERATION_FABRIC
0x000B  RESOURCE_MANAGEMENT_FABRIC
0x000C  RESOURCE_TRANSPORT_FABRIC

0x0010  RESOURCE_SUPERVISOR
0x0011  RESOURCE_HEALTH_MONITOR
0x0012  RESOURCE_PROVISIONING_ENGINE
0x0013  RESOURCE_VALIDATION_ENGINE
```

## Instrumentation Resources

```text
0x1000  RESOURCE_LOGIC_ANALYZER
0x1001  RESOURCE_OSCILLOSCOPE
0x1002  RESOURCE_DSP
0x1003  RESOURCE_PATTERN_GENERATOR
0x1004  RESOURCE_PROTOCOL_DECODER
0x1005  RESOURCE_COUNTER_BANK
0x1006  RESOURCE_TRIGGER_ROUTER
0x1007  RESOURCE_TRACE_BUFFER
```

## CPU Resources

```text
0x2000  RESOURCE_CPU
0x2001  RESOURCE_CPU_DIAGNOSTICS
0x2002  RESOURCE_CPU_TRACE
0x2003  RESOURCE_CPU_HEALTH
0x2004  RESOURCE_CPU_BOOT_STATUS
```

## Network Resources

```text
0x3000  RESOURCE_NETWORK_INTERFACE
0x3001  RESOURCE_PACKET_ENGINE
0x3002  RESOURCE_NETWORK_BOOT
```

## Storage Resources

```text
0x4000  RESOURCE_BLOCK_STORAGE
0x4001  RESOURCE_FILE_STORAGE
0x4002  RESOURCE_ROM_STORAGE
0x4003  RESOURCE_CONFIGURATION_STORAGE
```

## Accelerator Resources

```text
0x6000  RESOURCE_GPU
0x6001  RESOURCE_AI_ACCELERATOR
0x6002  RESOURCE_VECTOR_ENGINE
0x6003  RESOURCE_FPGA_COMPUTE
0x6004  RESOURCE_CRYPTO_ENGINE
```

## Experimental / Homebrew Resources

```text
0x8000 - 0xFFFF
```

Reserved for:

- prototype cards
- homebrew hardware
- experimental firmware
- development systems

No central registry assignment is required for this range.

Products intended for general release should migrate to assigned Resource Type IDs.

## Ring Security Notes

Resource Type IDs may be associated with default ownership, visibility, and control rings in future security policy documents.

Examples:

```text
RESOURCE_SUPERVISOR
    Owner:   Ring -2
    Visible: Ring -2 / Ring -1
    Control: Ring -2

RESOURCE_PATTERN_GENERATOR
    Owner:   Ring -2 / Ring -1
    Visible: Ring -2 / Ring -1 / Ring 0 by policy
    Control: Ring -2 / Ring -1 unless delegated
```

These notes are advisory in this registry.

Actual authorization belongs to capability and ring policy.

## Allocation Policy

Assigned IDs shall not be reused.

Deprecated IDs remain reserved permanently.

Resource Type IDs are allocated by registry update.

## Related Documents

- discovery-rom-format-v1.md
- service-id-registry-v1.md
- ADR-RING-SECURITY-MODEL.md
- capability-record-format-v1.md
- card-health-model-v1.md

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/ATX-SPEC-015-Reconciliation-Review.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/ATX-SPEC-015-Reconciliation-Review.md -->
### Integrated source: `docs/reviews/ATX-SPEC-015-Reconciliation-Review.md`

# ATX-SPEC-015 Reconciliation Review

## Status

Review complete

## Scope

This review reconciles ATX-SPEC-015 POSIX Compatibility Model against the architecture areas it most directly touches:

```text
ATX-SPEC-005 Mailbox Model
ATX-SPEC-010 Audit Model
ATX-SPEC-011 Error Model
ATX-SPEC-018 Recovery and Reconciliation Model
ATX-SPEC-019 Service Model
ATX-SPEC-021 Memory and Data Movement Model
```

The goal is to verify that POSIX compatibility remains a compatibility projection and does not become native Atarix authority.

## Review Result

```text
Overall Result: ACCEPTED WITH FOLLOW-UP REGISTRIES
```

ATX-SPEC-015 is coherent with the current Atarix architecture baseline.

No blocking conflict was identified.

Several follow-up registries should be created before implementation begins.

## Core Compatibility Rule Confirmed

ATX-SPEC-015 correctly establishes:

```text
POSIX is a compatibility personality.
POSIX is not native authority.
```

This aligns with the architecture-wide principles:

```text
Discovery is not authority.
Lookup is not access.
Observation is not control.
Persistence is not authority.
Recovery is not administrative access.
Compatibility is not authority.
```

## Reconciliation Against ATX-SPEC-005 Mailbox Model

ATX-SPEC-015 now states:

```text
Native IPC is mailbox-based.
Compatibility IPC is a projection.
```

This is consistent with the Atarix mailbox-centered IPC architecture.

POSIX-facing events, streams, queues, and local endpoints are modeled as projections over mailboxes or mailbox-backed service endpoints.

### Result

```text
Aligned
```

### Follow-Up

Define a compatibility IPC mapping table:

```text
Compatibility event -> mailbox event
Compatibility stream -> mailbox-backed stream endpoint
Compatibility queue -> mailbox queue
Compatibility local endpoint -> mailbox-backed local service endpoint
Network descriptor -> Network Service endpoint plus mailbox-backed descriptor
```

## Reconciliation Against ATX-SPEC-010 Audit Model

ATX-SPEC-015 correctly requires audit records to preserve both:

```text
Compatibility view
Native Atarix authority decision
```

The model also requires native error context to survive compatibility projection.

This prevents POSIX-style simplified return codes from erasing security-relevant audit context.

### Result

```text
Aligned
```

### Follow-Up

Add POSIX compatibility entries to the audit event registry:

```text
POSIX_ENV_CREATED
POSIX_PROFILE_SELECTED
POSIX_PATH_LOOKUP
POSIX_DESCRIPTOR_OPENED
POSIX_DESCRIPTOR_DUPLICATED
POSIX_DESCRIPTOR_INHERITED
POSIX_PROGRAM_LOADED
POSIX_PROCESS_CREATED
POSIX_IPC_EVENT_SENT
POSIX_NETWORK_ENDPOINT_CREATED
POSIX_ERROR_MAPPED
POSIX_RECOVERY_STARTED
POSIX_RECOVERY_RECONCILED
POSIX_ENV_QUARANTINED
```

## Reconciliation Against ATX-SPEC-011 Error Model

ATX-SPEC-015 correctly treats compatibility errors as projections of native Atarix errors.

The native error remains authoritative.

The specification explicitly states that unknown native errors must not be silently converted to success.

### Result

```text
Aligned
```

### Follow-Up

Create a POSIX compatibility error mapping table.

The table should preserve:

```text
Native error code
Compatibility error code
Object or service identity
Operation
Policy result
Lifecycle state
Audit event identity
```

## Reconciliation Against ATX-SPEC-018 Recovery and Reconciliation Model

ATX-SPEC-015 correctly states:

```text
Compatibility recovery restores execution.
Compatibility recovery does not restore authority.
```

Recovery validates descriptors, paths, processes, network endpoints, audit continuity, and quarantine outcomes against native Atarix state.

This is aligned with ATX-SPEC-018's recovery principle that recovery restores trust by proving consistency, not by assuming it.

### Result

```text
Aligned
```

### Follow-Up

Add POSIX recovery tests to the recovery test plan:

```text
Descriptor reconciliation fails closed
Path reconciliation does not grant access
Process reconciliation requires native lifecycle validity
Network reconciliation detects broken endpoints
Audit discontinuity becomes explicit
Quarantine blocks normal execution
Unknown recovery outcome fails closed
```

## Reconciliation Against ATX-SPEC-019 Service Model

ATX-SPEC-015 models device-like endpoints, network endpoints, storage-like endpoints, terminal-like endpoints, and entropy-like endpoints as service-bound compatibility endpoints rather than native device authority.

This is aligned with the Service Model principle that service location is not service identity.

Compatibility endpoint names are bindings to native services.

### Result

```text
Aligned
```

### Follow-Up

Define the initial service binding map:

```text
Null-like endpoint -> Null Service
Entropy-like endpoint -> Entropy Service
Terminal-like endpoint -> Terminal Service
Storage-like endpoint -> Storage Service
Network-like endpoint -> Network Service
```

## Reconciliation Against ATX-SPEC-021 Memory and Data Movement Model

ATX-SPEC-015 defers shared memory and memory mapping semantics to ATX-SPEC-021.

This is the correct architectural placement.

Shared memory must not become a path around capability checks, policy checks, audit, or recovery boundaries.

### Result

```text
Aligned, pending ATX-SPEC-021
```

### Follow-Up

When ATX-SPEC-021 is drafted, include a section for POSIX shared memory projection.

## Findings

### Finding 001: POSIX Compatibility Is Properly Contained

ATX-SPEC-015 consistently treats POSIX as a projection over native Atarix semantics.

This prevents POSIX identity, paths, descriptors, and process identifiers from becoming native authority.

### Finding 002: Mailbox-Centered IPC Is The Correct Native Mapping

ATX-SPEC-015 avoids creating separate native subsystems for POSIX events, streams, queues, local endpoints, and network descriptors.

This keeps authority, audit, recovery, and resource accounting unified.

### Finding 003: Error And Audit Registries Are Now Required

The POSIX model is coherent, but implementation should not begin until compatibility audit events and error mappings are registered.

### Finding 004: Profiles Need Object Format

The profile model is sound, but the profile registry needs a concrete object format before implementation.

### Finding 005: Shared Memory Must Wait For ATX-SPEC-021

The deferment of shared memory semantics is correct.

Implementing compatibility shared memory before ATX-SPEC-021 would risk creating unreviewed authority and data movement behavior.

## Required Follow-Up Work

```text
1. Create POSIX compatibility profile registry object format.
2. Create POSIX compatibility error mapping table.
3. Add POSIX compatibility audit event identifiers.
4. Define compatibility IPC mapping table against ATX-SPEC-005.
5. Define initial service binding map against ATX-SPEC-019.
6. Add POSIX recovery tests against ATX-SPEC-018.
7. Defer shared memory semantics to ATX-SPEC-021.
```

## Review Decision

ATX-SPEC-015 is accepted as coherent with the current architecture baseline.

It should remain marked as draft until the follow-up registries and mapping tables are created.

## Summary

ATX-SPEC-015 successfully preserves the central Atarix architecture rule:

```text
Compatibility is not authority.
```

The specification is ready to inform implementation planning after registries are created for profiles, errors, audit events, IPC mappings, and service bindings.

<!-- AEMS-MIGRATED-SOURCE: docs/reviews/capability-sprint-1-review.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/reviews/capability-sprint-1-review.md -->
### Integrated source: `docs/reviews/capability-sprint-1-review.md`

# Capability Sprint 1 Review

Status: Complete

## Objective

Capability Sprint 1 established the first tested capability-policy foundation for ATARIX.

The sprint translated the accepted capability evaluation model into public headers, implementation code, and automated regression tests.

## Implemented Components

Public API:

```text
include/atarix/capability.h
include/atarix/capability_engine.h
include/atarix/capability_policy.h
```

Implementation:

```text
src/capability/capability_evaluator.c
src/capability/capability_policy.c
```

Tests:

```text
tests/capability/test_capability_validation.c
tests/capability/test_capability_ownership.c
tests/capability/test_capability_ring_boundary.c
tests/capability/test_capability_delegation.c
tests/capability/test_capability_revocation.c
```

## Security Invariants Verified

Capability Sprint 1 verifies the following security invariants:

```text
Validation
Ownership
Ring boundaries
Delegation
Revocation
Expiration
Deny-by-default behavior
```

## Validation

Capability records are rejected when structurally invalid.

The validation tests cover:

```text
NULL capability records
Invalid capability identifiers
Invalid operations
Invalid ring ranges
Missing target handles
Missing issuer handles
```

## Ownership

Capabilities are principal-bound.

The ownership tests cover:

```text
Caller-owner match allowed
Caller-owner mismatch denied
Revoked owner trust denied
Untrusted issuer denied
```

## Ring Boundaries

Ring boundaries are enforced as absolute authorization boundaries.

The ring tests cover:

```text
Application self-access
Service self-access
Driver self-access
Kernel self-access
Fabric self-access
Supervisor self-access
Application denied Kernel authority
Service denied Driver authority
Driver denied Fabric authority
Kernel denied Supervisor authority
```

## Delegation

Delegation may reduce authority only.

The delegation tests cover:

```text
Kernel to Driver allowed
Driver to Service allowed
Service to Application allowed
Application to Service denied
Service to Driver denied
Driver to Kernel denied
Delegation without delegation flag denied
Supervisor authority delegation denied
Delegation that adds flags denied
```

## Revocation and Expiration

Revocation and expiration are enforced before ordinary trust, ownership, ring, resource, and operation checks.

The revocation tests cover:

```text
Valid capability allowed
Revoked capability denied
Expired capability denied
Expiration boundary allowed
Zero lifetime denied
Revocation overrides ownership
Revocation overrides trust
```

## CI Result

Capability Sprint 1 completed with the following automated test result:

```text
TOTAL: 14
PASS:  14
SKIP:  0
XFAIL: 0
FAIL:  0
XPASS: 0
ERROR: 0
```

Test inventory:

```text
Discovery tests:   9
Capability tests:  5
Total tests:      14
```

## Architectural Decisions Implemented

The sprint implements the capability evaluation model established by:

```text
docs/adr/ADR-00X-capability-evaluation-model.md
```

Core decisions implemented:

```text
Supervisor is the root of trust.
Fabric is the enforcement authority.
Capabilities are principal-bound.
Capabilities are non-transferable.
Delegation may only reduce authority.
Revocation is authoritative.
Expiration is enforced.
Default decision is DENY.
```

## Deferred Work

The following work is intentionally deferred to later capability or directory sprints:

```text
Global atarix_status_t
Capability issuance workflow
Capability chain storage
Capability revocation registry
Supervisor-to-Fabric revocation propagation
Cryptographic capability integrity
Audit log integration
Capability serialization hardening
Cross-subsystem Directory integration
```

## Sprint 2 Recommendations

Recommended next capability work:

```text
Define capability issuance semantics.
Add capability attenuation helpers.
Define revocation registry representation.
Integrate capability policy with Directory Sprint 1.
Introduce global status handling when statuses cross subsystem boundaries.
Add audit-friendly reason codes.
```

## Result

Capability Sprint 1 is complete.

The ATARIX repository now has a tested capability-policy baseline with validation, ownership, ring, delegation, revocation, and expiration coverage.

<!-- AEMS-MIGRATED-SOURCE: docs/ring-security-test-plan.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/ring-security-test-plan.md -->
### Integrated source: `docs/ring-security-test-plan.md`

# ATARIX Ring Security Test Plan

Status: Planned

## Purpose

This document records the test additions required to make ATARIX ring security executable rather than merely documented.

Existing tests should not be rewritten. The ring-security work adds new tests that prove ring policy is enforced before capability authority, mailbox delivery, fabric routing, and Supervisor service access.

## Security Model Under Test

ATARIX uses two related but separate protection mechanisms:

```text
Ring security       = coarse trust boundary
Capability security = fine-grained authority boundary
```

The ring boundary is the large-gauge net.

Capabilities are the fine mesh.

A capability may reduce, delegate, expire, or revoke authority, but it may not grant authority that violates ring policy.

Required decision chain:

```text
Deny by default
    -> Ring boundary check
    -> Capability validation
    -> Revocation check
    -> Mailbox validation
    -> Service policy
    -> Resource policy
    -> Authorized operation
```

Failure at any step denies the operation.

## Baseline Ring Model

Initial rings:

```text
Ring 0: Supervisor / Root Authority
Ring 1: Fabric Management / Trusted System Services
Ring 2: Device Services / Card Firmware
Ring 3: Applications / User Workloads
Ring 4: Untrusted or Quarantined Nodes
```

The exact ring set may evolve by ADR. The existence of ring boundaries is mandatory.

## Current Tests That Remain Valid

The following existing test families remain valid and should not be rewritten simply because ring policy is being added:

```text
discovery/*
capability/*
mailbox/*
sim/*
```

Ring security adds new test coverage. It does not invalidate the current 29-test baseline.

## New Ring Manager Tests

Planned test files:

```text
tests/ring/test_ring_self_access.c
tests/ring/test_ring_upward_denied.c
tests/ring/test_ring_downward_allowed.c
tests/ring/test_ring_quarantine_denied.c
tests/ring/test_ring_supervisor_override.c
```

### test_ring_self_access

Proves that an actor may access resources within its own ring when operation policy allows it.

Expected behavior:

```text
Ring N -> Ring N: allowed for permitted operation classes
```

### test_ring_upward_denied

Proves lower-trust rings cannot directly command, inspect, mutate, bypass, or impersonate higher-trust rings.

Expected behavior:

```text
Ring 3 -> Ring 0: denied
Ring 2 -> Ring 0: denied unless explicitly modeled as a supervised request path
Ring 4 -> any trusted ring: denied
```

### test_ring_downward_allowed

Proves higher-trust rings may supervise, constrain, or revoke authority from lower-trust rings.

Expected behavior:

```text
Ring 0 -> Ring 1: allowed for Supervisor operations
Ring 0 -> Ring 2: allowed for Supervisor operations
Ring 1 -> Ring 2: allowed for Fabric-management operations
```

### test_ring_quarantine_denied

Proves quarantined nodes cannot reach trusted services or other normal nodes.

Expected behavior:

```text
Ring 4 -> Ring 0: denied
Ring 4 -> Ring 1: denied
Ring 4 -> Ring 2: denied
Ring 4 -> Ring 3: denied unless explicit quarantine-service policy allows it
```

### test_ring_supervisor_override

Proves Ring 0 can perform recovery, revocation, and quarantine actions that lower rings cannot perform.

Expected behavior:

```text
Ring 0 may quarantine lower rings.
Ring 0 may revoke lower-ring authority.
Ring 0 may inspect Supervisor-owned tables.
Lower rings may not override Ring 0 policy.
```

## Capability/Ring Interaction Tests

Planned test files:

```text
tests/capability/test_capability_respects_ring_boundary.c
tests/capability/test_delegation_cannot_cross_ring_boundary.c
tests/capability/test_revoked_capability_still_denied_after_ring_check.c
```

### test_capability_respects_ring_boundary

Proves a syntactically valid capability is still denied if ring policy forbids the request.

Example:

```text
Ring 3 application holds a valid-looking capability for a Ring 0 endpoint.
Ring policy denies before resource access.
```

### test_delegation_cannot_cross_ring_boundary

Proves delegation cannot increase authority across a prohibited ring boundary.

Example:

```text
Ring 2 device service delegates to Ring 3 application.
Delegated capability may only reduce authority.
Delegation cannot grant Ring 3 direct access to Ring 1 or Ring 0 internals.
```

### test_revoked_capability_still_denied_after_ring_check

Proves revocation remains effective even when ring policy would otherwise allow the request.

Example:

```text
Ring 3 -> Ring 2 permitted by ring policy.
Capability exists but is revoked.
Operation is denied.
```

## Mailbox/Ring Interaction Tests

Planned test files:

```text
tests/mailbox/test_mailbox_ring_restricted.c
tests/mailbox/test_mailbox_supervisor_endpoint_denied.c
tests/mailbox/test_mailbox_quarantine_isolation.c
```

### test_mailbox_ring_restricted

Proves mailbox delivery cannot bypass ring policy.

Valid mailbox structure is not enough.

### test_mailbox_supervisor_endpoint_denied

Proves lower rings cannot send arbitrary commands to Supervisor-owned mailbox endpoints.

Supervisor requests must go through explicit service policy.

### test_mailbox_quarantine_isolation

Proves quarantined nodes cannot use otherwise valid mailbox traffic to reach trusted nodes.

## Fabric/Ring Interaction Tests

Planned test files:

```text
tests/sim/test_fabric_ring_routing.c
tests/sim/test_fabric_quarantine_node.c
tests/sim/test_fabric_supervisor_visibility.c
```

### test_fabric_ring_routing

Proves fabric routing consults ring policy before delivery.

A valid route is not sufficient if the ring boundary denies communication.

### test_fabric_quarantine_node

Proves the Supervisor can quarantine a node and the fabric prevents normal traffic to or from that node.

### test_fabric_supervisor_visibility

Proves Ring 0 can observe fabric topology while lower rings see only what service policy exposes.

## Suggested Test Count Milestones

Current baseline:

```text
29 tests passing
```

Ring Manager milestone:

```text
+5 ring tests
34 tests expected
```

Capability/ring interaction milestone:

```text
+3 capability/ring tests
37 tests expected
```

Mailbox/ring interaction milestone:

```text
+3 mailbox/ring tests
40 tests expected
```

Fabric/ring interaction milestone:

```text
+3 fabric/ring tests
43 tests expected
```

## Implementation Ordering

Recommended order:

```text
1. Ring Manager public interface
2. Ring Manager core tests
3. Capability/ring integration
4. Mailbox/ring integration
5. Fabric/ring integration
6. Supervisor service registry
```

Do not build Supervisor service registration before ring policy has executable tests.

## Result

This test plan makes the ring-security doctrine actionable.

Ring security becomes a required, testable boundary rather than an implied design intention.

<!-- AEMS-MIGRATED-SOURCE: docs/routing-design-notes.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/routing-design-notes.md -->
### Integrated source: `docs/routing-design-notes.md`

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

<!-- AEMS-MIGRATED-SOURCE: docs/security.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/security.md -->
### Integrated source: `docs/security.md`

# ATARIX Security Architecture

## Status

Draft security architecture specification.

This document defines the ATARIX security model, including privilege rings, capability-mediated access, DMA controls, device trust boundaries, accelerator isolation, supervisor recovery, and future multiprocessor considerations.

## Principle

Devices are not trusted by default.

Security is designed into the architecture rather than added later.

ATARIX uses two complementary concepts:

```text
Privilege Rings
    define execution authority.

Capabilities
    define resource authority.
```

A privilege ring answers:

```text
What kind of code is this?
```

A capability answers:

```text
What specific resource may this code or device access?
```

Both are required for a complete security model.

## Privilege Ring Model

ATARIX should define four logical privilege rings.

These rings are inspired by conventional Ring 0 through Ring 3 operating-system models, but they must be adapted to the W65C816 and FPGA-fabric environment.

The W65C816 does not provide native x86-style hardware rings. Therefore, rings are initially a software and fabric-enforced design discipline, with increasing hardware assistance added over time.

## Ring 0 - Kernel and Core Authority

Ring 0 is the most privileged normal execution ring.

Responsibilities:

- Kernel execution.
- Memory-management policy.
- Capability broker.
- Device-driver authority assignment.
- Interrupt-core management.
- DMA grant creation.
- Fabric configuration policy.

Ring 0 may request privileged fabric operations, but it should still use explicit capability records rather than relying on implicit trust.

Ring 0 does not replace the supervisor management plane.

## Ring 1 - System Services and Trusted Drivers

Ring 1 contains trusted but separable system services.

Examples:

- Filesystem service.
- Network service.
- Storage service.
- Console service.
- Trusted block-device drivers.
- Memory-service manager.

Ring 1 code may receive broad capabilities, but it should not receive unrestricted access by default.

Ring 1 services should be restartable where practical.

## Ring 2 - Device Services and Semi-Trusted Runtime Components

Ring 2 contains semi-trusted services and device-facing components.

Examples:

- Specific hardware drivers.
- Accelerator runtime services.
- Protocol workers.
- Debug agents.
- User-session support services.

Ring 2 components should receive narrow capabilities tied to specific devices, buffers, mailboxes, and interrupts.

A Ring 2 failure should not normally crash the whole machine.

## Ring 3 - User Programs and Untrusted Code

Ring 3 contains ordinary user programs and experimental code.

Ring 3 code should not directly access:

- Raw device registers.
- DMA engines.
- Fabric configuration registers.
- Supervisor controls.
- Privileged interrupt routing.
- Unmediated memory windows.

Ring 3 code must request services through system calls, mailboxes, or mediated service APIs.

## Supervisor Plane - Outside the Ring Model

The supervisor controller is outside the normal Ring 0 through Ring 3 execution model.

The supervisor is a management and recovery authority, not an application execution ring.

Responsibilities:

- RTC.
- Watchdogs.
- Reset control.
- Fault logging.
- Power-good monitoring.
- Slot quarantine.
- Recovery-mode entry.
- Golden ROM or golden bitstream selection.

The supervisor should not silently perform normal data-plane work.

The supervisor may reset or quarantine Ring 0 failures, fabric failures, CPU-card failures, or device failures.

See:

```text
docs/supervisor-card-v1.md
docs/management-anomaly-detection-v1.md
```

## Capability Model

Capabilities define specific resource authority.

Capabilities describe:

- Resource ownership.
- Allowed operations.
- Address windows.
- DMA permissions.
- Mailbox permissions.
- Device permissions.
- Revocation policy.

Capabilities prevent the ring model from becoming too coarse.

For example, two Ring 1 services may both be trusted services, but only the storage service should receive access to storage-controller DMA buffers.

See:

```text
docs/capability-model.md
```

## Ring and Capability Interaction

Access decisions should consider both ring and capability.

Example policy:

```text
Ring 0 + valid fabric capability
    -> may configure fabric service

Ring 1 + valid device capability
    -> may operate assigned device

Ring 2 + valid mailbox capability
    -> may submit commands to assigned service

Ring 3 + no capability
    -> denied
```

Being in a privileged ring is not enough to access every resource.

Having a capability is not enough if the caller is executing in a ring that is not allowed to exercise that kind of capability.

## DMA Security

DMA access must be mediated.

DMA requests require:

1. Valid DMA capability.
2. Valid memory capability.
3. Valid ownership chain.
4. Ring policy allowing the request.

Future FPGA fabric components may participate in:

- DMA authorization.
- DMA isolation.
- DMA revocation.
- Fault reporting.
- Timeout enforcement.

No device should receive unrestricted DMA access to system memory.

See:

```text
docs/dma-engine-v1.md
```

## Device Trust Model

Devices receive explicit access grants.

No device should automatically gain unrestricted memory visibility or register access simply because it is physically installed.

Device discovery should describe:

- Device identity.
- Required capabilities.
- DMA needs.
- Mailbox endpoints.
- Interrupt sources.
- Driver binding requirements.

See:

```text
docs/discovery-rom-format.md
```

## Fabric Security

The FPGA fabric is an enforcement assistant.

The fabric may implement:

- Register access filtering.
- DMA bounds checking.
- Interrupt routing control.
- Mailbox endpoint control.
- Discovery visibility.
- Fault counters.
- Capability lookup acceleration.

The fabric should not become an undocumented black box.

Security-relevant fabric behavior must be described by stable register, mailbox, and discovery specifications.

## Multiprocessor Security

Future CPU modules should communicate through mediated interfaces.

Examples:

- Mailboxes.
- Shared queues.
- Controlled memory windows.
- Capability-backed DMA buffers.

Each CPU card should retain local bank-zero isolation.

A future secondary CPU should not automatically obtain Ring 0 authority merely because it is physically installed.

CPU-card identity, boot state, and assigned authority should be explicit.

## Accelerator Security

Accelerators are treated as powerful but untrusted participants.

They receive explicit capabilities and controlled access to resources.

Accelerator jobs should follow this pattern:

```text
1. Discover accelerator.
2. Allocate buffers.
3. Grant narrow memory capability.
4. Grant accelerator execution capability.
5. Submit job through mailbox or command queue.
6. Validate completion.
7. Revoke temporary capabilities.
```

Accelerators should never receive unrestricted access to CPU-local memory or kernel memory.

## Recovery and Fault Isolation

Security failures should degrade safely.

Possible recovery actions:

- Revoke capability.
- Disable DMA channel.
- Reset service.
- Reset slot.
- Quarantine card.
- Enter recovery mode.
- Preserve fault log for diagnosis.

The supervisor controller should assist recovery and fault isolation.

## Rev A Enforcement Strategy

Rev A does not need to implement full hardware isolation.

Rev A should establish the policy structure and make violations visible.

Recommended Rev A behavior:

- Define Rings 0-3 in software architecture.
- Keep supervisor outside the ring model.
- Require capabilities in firmware interfaces where practical.
- Prevent unrestricted DMA.
- Log denied or invalid accesses.
- Keep dangerous features disabled by default.
- Provide recovery mode.

## Future Enforcement Strategy

Later revisions may add:

- FPGA access-control tables.
- Hardware-assisted DMA bounds checking.
- MMU or window-mapping experiments.
- Per-ring memory windows.
- Capability-tagged mailbox endpoints.
- ABORT-based protection faults.
- Slot quarantine.

## Design Principle

ATARIX should not rely on a single security mechanism.

Use rings for broad execution privilege.

Use capabilities for precise resource authority.

Use the supervisor for recovery and accountability.

Use the FPGA fabric for mediation and enforcement assistance.

## Open Questions

- How Ring 0 through Ring 3 transitions are represented in the OS.
- Whether Ring 3 is enforced by software convention, memory windows, or future MMU logic.
- Whether ABORT should signal protection violations.
- How capabilities are stored and revoked.
- Whether capability tables live in system RAM, supervisor memory, or fabric memory.
- How secondary CPU cards acquire initial authority.
- Whether signed boot images are required before multi-user operation.

<!-- AEMS-MIGRATED-SOURCE: docs/service-directory-format-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/service-directory-format-v1.md -->
### Integrated source: `docs/service-directory-format-v1.md`

# Service Directory Format v1

Status: Draft v1

## Purpose

The Service Directory maps discovered and authorized services to bindable runtime entries.

The Service Directory answers:

```text
Where is it?
Who provides it?
Which handle implements it?
How do I bind to it?
```

## Architectural Chain

```text
Identity
    identifies

Discovery
    describes

Trust
    evaluates

Capabilities
    authorize

Rings
    constrain

Directory
    locates

Transport
    operates
```

## Non-Goals

The Service Directory does not:

- identify hardware
- establish trust
- grant capabilities
- override ring policy
- replace supervisor safety authority

## Directory Header

```text
Offset  Size  Field
------  ----  ----------------------
0x00    4     Magic
0x04    2     Version Major
0x06    2     Version Minor
0x08    4     Total Length
0x0C    4     Entry Count
0x10    4     Flags
0x14    4     Header CRC32
0x18    4     Directory CRC32
0x1C    4     Reserved
```

Magic:

```text
ATDR
```

Meaning:

```text
ATARIX Directory
```

Header size:

```text
32 bytes
```

## Directory Entry

All entries are 8-byte aligned.

```text
Offset  Size  Field
------  ----  ----------------------
0x00    8     Service Handle
0x08    8     Provider Handle
0x10    4     Service ID
0x14    4     Provider ID
0x18    4     Flags
0x1C    4     Binding Type
0x20    8     Binding Handle
0x28    8     Required Capability Handle
0x30    4     Minimum Ring
0x34    4     Reserved
```

Entry size:

```text
56 bytes
```

## Binding Type

Initial binding types:

```text
0x00000000  Invalid
0x00000001  Mailbox
0x00000002  Transport handle
0x00000003  Shared memory
0x00000004  Supervisor call
0x00000005  Fabric service
0x00000006  Network endpoint
0x00000007  Storage endpoint
```

## Flags

Initial flags:

```text
Bit 0   Available
Bit 1   Authorized
Bit 2   Restricted
Bit 3   Recovery only
Bit 4   Development only
Bit 5   Hotplug capable
Bit 6   Instrumented
Bits 7-31 reserved
```

## Directory Construction

Recommended construction sequence:

```text
Identity validation
    ↓
Discovery enumeration
    ↓
Trust evaluation
    ↓
Capability evaluation
    ↓
Ring policy filtering
    ↓
Directory construction
```

The Directory should contain the authorized platform view appropriate to the requesting ring.

## Ring Filtering

Different rings may receive different directory views.

Example:

```text
Ring -2
    Full supervisor view

Ring -1
    Platform firmware view

Ring 0
    Kernel-authorized services

Ring 3
    Application-safe services only
```

## Recovery Rule

Recovery must not depend on Directory Services.

The supervisor must retain direct diagnostic access when the directory is unavailable, corrupted, or incomplete.

## Security Rules

Directory entries do not grant authority.

Directory entries describe authorized binding results after capability and ring policy have already been evaluated.

## Related Documents

- discovery-rom-format-v1.md
- service-id-registry-v1.md
- capability-record-format-v1.md
- trust-model-v1.md
- ADR-RING-SECURITY-MODEL.md

<!-- AEMS-MIGRATED-SOURCE: docs/service-id-registry-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/service-id-registry-v1.md -->
### Integrated source: `docs/service-id-registry-v1.md`

# Service ID Registry v1

Status: Draft v1

## Purpose

This document defines the canonical ATARIX Service ID registry.

Service IDs identify software-visible services exposed through the Enumeration Fabric and later bound through Directory Services and Transport mechanisms.

Service IDs are used by:

- Discovery ROM
- Service Directory
- Capability Records
- Backplane BIOS
- Monitor
- Operating system services
- Supervisor and instrumentation tooling

A Service ID identifies a class of service.

A Service ID does not identify a specific instance.

Specific instances are identified through handles and directory records.

## Architectural Rules

```text
Identity identifies.
Enumeration describes.
Trust evaluates.
Capabilities authorize.
Rings constrain authority.
Directory locates.
Transport operates.
```

Service IDs do not grant authority.

Service IDs do not imply trust.

Service IDs do not imply capability.

## Service ID Ranges

```text
0x0000          Reserved / invalid
0x0001-0x0FFF   Core platform services
0x1000-0x1FFF   Instrumentation services
0x2000-0x2FFF   CPU services
0x3000-0x3FFF   Network services
0x4000-0x4FFF   Storage services
0x5000-0x5FFF   Reserved
0x6000-0x6FFF   Accelerator services
0x7000-0x7FFF   Reserved
0x8000-0xFFFF   Experimental / homebrew services
```

## Core Platform Services

```text
0x0001  SERVICE_DISCOVERY
0x0002  SERVICE_DIRECTORY
0x0003  SERVICE_PROVISIONING
0x0004  SERVICE_VALIDATION
0x0005  SERVICE_MONITOR
0x0006  SERVICE_HEALTH
0x0007  SERVICE_FAULT_LOG
0x0008  SERVICE_EVENT_LOG
0x0009  SERVICE_TIMEBASE
0x000A  SERVICE_MAILBOX
0x000B  SERVICE_DMA
0x000C  SERVICE_INTERRUPT_ROUTING
0x000D  SERVICE_RECOVERY
```

## Instrumentation Services

```text
0x1000  SERVICE_INSTRUMENTATION
0x1001  SERVICE_CAPTURE
0x1002  SERVICE_ANALYSIS
0x1003  SERVICE_TRACE
0x1004  SERVICE_LOGIC_ANALYZER
0x1005  SERVICE_OSCILLOSCOPE
0x1006  SERVICE_PATTERN_GENERATOR
0x1007  SERVICE_PROTOCOL_DECODER
0x1008  SERVICE_COUNTERS
0x1009  SERVICE_TRIGGER_ROUTER
```

## CPU Services

```text
0x2000  SERVICE_CPU_DIAGNOSTICS
0x2001  SERVICE_CPU_HEALTH
0x2002  SERVICE_CPU_TRACE
0x2003  SERVICE_CPU_BOOT_STATUS
0x2004  SERVICE_CPU_VALIDATION
```

## Network Services

```text
0x3000  SERVICE_NETWORK_INTERFACE
0x3001  SERVICE_NETWORK_BOOT
0x3002  SERVICE_PACKET_TRANSPORT
0x3003  SERVICE_TIME_SYNC
```

## Storage Services

```text
0x4000  SERVICE_BLOCK_STORAGE
0x4001  SERVICE_FILE_STORAGE
0x4002  SERVICE_ROM_STORAGE
0x4003  SERVICE_CONFIGURATION_STORAGE
```

## Accelerator Services

```text
0x6000  SERVICE_GPU_COMPUTE
0x6001  SERVICE_AI_ACCELERATION
0x6002  SERVICE_VECTOR_COMPUTE
0x6003  SERVICE_FPGA_COMPUTE
0x6004  SERVICE_CRYPTO_ACCELERATION
```

## Experimental / Homebrew Services

```text
0x8000 - 0xFFFF
```

Reserved for:

- prototype services
- homebrew services
- experimental firmware
- development systems

No central registry assignment is required for this range.

Products intended for general release should migrate to assigned Service IDs.

## Ring Security Notes

Service IDs may be associated with default ownership, visibility, and control rings in future security policy documents.

Examples:

```text
SERVICE_RECOVERY
    Owner:   Ring -2
    Visible: Ring -2 / Ring -1
    Control: Ring -2

SERVICE_PATTERN_GENERATOR
    Owner:   Ring -2 / Ring -1
    Visible: Ring -2 / Ring -1 / Ring 0 by policy
    Control: Ring -2 / Ring -1 unless delegated

SERVICE_CPU_HEALTH
    Owner:   Ring -2 / Ring -1
    Visible: Ring -2 / Ring -1 / Ring 0
    Control: Ring -2 / Ring -1
```

These notes are advisory in this registry.

Actual authorization belongs to capability and ring policy.

## Allocation Policy

Assigned IDs shall not be reused.

Deprecated IDs remain reserved permanently.

Service IDs are allocated by registry update.

## Related Documents

- discovery-rom-format-v1.md
- resource-type-registry-v1.md
- ADR-RING-SECURITY-MODEL.md
- capability-record-format-v1.md
- service-directory-format-v1.md

<!-- AEMS-MIGRATED-SOURCE: docs/service-model.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/service-model.md -->
### Integrated source: `docs/service-model.md`

# ATARIX Service Model

Status: Baseline

## Purpose

This document defines the ATARIX service model.

ATARIX is service-addressed, not address-addressed. Applications and higher-level software do not directly access hardware. They access services. Services operate on resources. Resources are implemented by concrete hardware, firmware, simulation, or future node implementations.

## Required Protected-Access Path

All protected access follows this path:

```text
Application
    -> Service
    -> Capability Policy
    -> Resource
    -> Implementation
```

This means:

```text
Applications request service operations.
Services validate capabilities.
Capabilities authorize resource access.
Resources hide implementation details.
Implementations touch hardware or simulated hardware.
```

## Forbidden Access Path

The following pattern is forbidden as a public software contract:

```text
Application
    -> Physical Address
    -> Register
    -> Hardware
```

Examples of forbidden direct access:

```c
*(volatile unsigned int *)0xF0001000u = value;
```

```c
write_register(0xD123u, value);
```

unless such code exists inside a resource implementation layer that is already protected by service and capability policy.

## Visibility Is Not Authority

Discovery grants visibility.

Directory resolves and describes.

Capabilities authorize.

Knowing any of the following does not grant access:

```text
Service name
Handle
Physical address
Register offset
Slot number
Mailbox endpoint
Discovery record
Directory entry
```

## Example: Storage

The caller sees:

```text
Storage Service
```

The protected path is:

```text
Application
    -> Storage Service
    -> Storage Capability
    -> Block Resource
    -> Storage Implementation
```

The implementation may be:

```text
CF card
SD card
SATA device
Network block service
Simulator file
Future Fabric storage card
```

The caller must not depend on the implementation.

## Example: Display

The caller sees:

```text
Display Service
```

The protected path is:

```text
Application
    -> Display Service
    -> Display Capability
    -> Framebuffer Resource
    -> Display Implementation
```

The implementation may be:

```text
FPGA framebuffer
Local video card
Remote display node
Simulator framebuffer
```

## Example: GPIO

The caller sees:

```text
GPIO Service
```

The protected path is:

```text
Application
    -> GPIO Service
    -> GPIO Capability
    -> GPIO Resource
    -> Pin Controller Implementation
```

The caller does not directly manipulate pin registers.

## Simulator Rule

The simulator follows the same model as hardware.

Simulator code must not introduce privileged shortcuts that would be invalid on real hardware.

A simulated resource is still a resource. It is not permission to bypass services or capabilities.

## Service Contracts

A service contract should define:

```text
Service name
Operation identifiers
Request format
Response format
Required capability
Failure statuses
Versioning rules
Observability requirements
```

## Security Requirements

All protected services must:

```text
Deny by default.
Validate capability before access.
Reject malformed requests.
Report explicit failures.
Avoid exposing implementation addresses.
Avoid treating handles as authority.
```

## Result

The ATARIX service model enforces abstraction and security.

Hardware may change. Simulators may replace hardware. Future nodes may provide services. The caller still uses the same service-oriented access path.

<!-- AEMS-MIGRATED-SOURCE: docs/software.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/software.md -->
### Integrated source: `docs/software.md`

# ATARIX Software Architecture

## Philosophy

Software should grow alongside the hardware.

The project begins with monitor firmware and eventually evolves into a workstation operating system.

## Development Stages

### Stage 1

- ROM monitor
- Serial console
- Memory diagnostics
- Device diagnostics

### Stage 2

- Program loader
- Modular firmware services
- Networking support

### Stage 3

- Kernel experimentation
- Driver framework
- Device discovery

### Stage 4

- Capability broker
- DMA manager
- Multiprocessor services

### Stage 5

- Accelerator runtime
- Distributed services
- Heterogeneous compute framework

## Toolchain

Candidate tools:

- 64TASS
- WDC tools
- LLVM research paths
- Open-source cross-development tools

## Development Environment

- Vim
- Make
- Git
- Automated testing
- FPGA simulation

## Long-Term Goals

- Native operating system
- Capability-oriented services
- Hardware-assisted security
- Modular driver architecture

<!-- AEMS-MIGRATED-SOURCE: docs/supervisor-card-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/supervisor-card-v1.md -->
### Integrated source: `docs/supervisor-card-v1.md`

# ATARIX Supervisor Card Architecture v1

## Status

Draft hardware architecture specification.

This document defines the first-generation ATARIX supervisor card. The supervisor card implements the independent management plane for the ATARIX workstation architecture.

## Purpose

The supervisor card keeps the system observable, recoverable, and serviceable even when the W65C816 CPU, FPGA fabric controller, service cards, or accelerator cards have failed or are held in reset.

The supervisor is not a replacement for the CPU or fabric controller. It is a management and recovery authority.

## Design Rule

The supervisor must remain operational when the CPU and fabric are not booting.

It owns recovery, reset sequencing, watchdogs, RTC timekeeping, power-state monitoring, and fault logging.

## Architectural Position

```text
RTC / NVRAM / Fault Log
        |
Supervisor MCU
        |
Management Plane
        |
CPU Card / ULX3S Fabric Controller / Backplane Slots
```

The supervisor participates in the management plane, not the primary data plane.

## Design Goals

1. Provide deterministic reset sequencing.
2. Provide RTC-backed timekeeping.
3. Timestamp fault logs.
4. Monitor CPU, FPGA, and slot heartbeats.
5. Reset hung cards or the full system.
6. Support golden ROM and golden FPGA recovery.
7. Expose diagnostic information over UART or management interface.
8. Remain independent from the main CPU and FPGA fabric.
9. Support NTP-corrected RTC updates when the system network is available.

## Required Functional Blocks

### Supervisor MCU

Candidate MCU families:

```text
RP2350
RP2040
STM32
ESP32-S3
```

Preferred attributes:

- Multiple GPIOs.
- I2C/SPI/UART support.
- Nonvolatile configuration support.
- Watchdog timers.
- Low-power standby capability.
- Good open toolchain support.

### RTC

The supervisor card should include a battery-backed or supercapacitor-backed RTC.

Responsibilities:

- Maintain time when the system is powered off.
- Timestamp watchdog resets.
- Timestamp recovery-mode entries.
- Timestamp slot failures.
- Provide fallback time when NTP is unavailable.

NTP may correct the RTC at boot, but NTP does not replace the RTC.

See:

```text
docs/netboot-ntp-v1.md
```

### Fault Log Storage

The supervisor should include nonvolatile or retained fault-log storage.

Candidate storage:

```text
MCU internal flash
External SPI flash
I2C EEPROM
FRAM
Battery-backed SRAM
```

FRAM is attractive for repeated fault logging because it tolerates many writes.

Fault log entries should include:

```text
Timestamp
Fault source
Fault class
Reset cause
Slot number
Last heartbeat state
Power-good state
Fabric status if available
CPU status if available
```

### Watchdog Timers

The supervisor should monitor heartbeats from:

```text
Primary CPU Card
ULX3S Fabric Controller
Service Cards
Accelerator Cards
Future CPU Cards
```

Watchdog action levels:

```text
Level 0 Log warning
Level 1 Assert slot fault
Level 2 Reset offending slot
Level 3 Reset fabric controller
Level 4 Reset CPU card
Level 5 Enter recovery mode
```

Not every fault should trigger a full system reset.

### Reset Controller

The supervisor should control:

```text
Global Reset
CPU Slot Reset
Fabric Controller Reset
Per-Slot Reset
Recovery Reset
Manual Reset Input
Watchdog Reset Output
```

The reset controller must support holding selected slots in reset while allowing the supervisor to continue operating.

### Power Monitoring

The supervisor should monitor:

```text
+12V
+5V
+3.3V
+3.3V Standby
Optional local rails
```

Power faults should be logged before reset or shutdown if possible.

### Recovery Inputs

Required recovery controls:

```text
Recovery Button
Recovery DIP Switch
Golden ROM Select
Golden FPGA Bitstream Select
Fabric Disable
DMA Disable
Slow Clock / Debug Clock Enable
```

These may be physically located on the supervisor card, backplane, or ULX3S carrier, but the supervisor should be able to read their state.

## Management Plane

The supervisor communicates with system components over a low-speed management interface.

Candidate electrical interfaces:

```text
I2C
SPI
UART multidrop
RS-485 multidrop
CAN
Custom low-speed management bus
```

The management plane should be independent of the main ATARIX data plane.

Management-plane functions:

```text
Heartbeat collection
Slot identity readout
Reset control
Fault reporting
RTC read/write
Configuration read/write
Power-state reporting
Recovery-mode coordination
```

## Heartbeat Model

Each active card should provide a heartbeat.

Heartbeat states:

```text
Absent
Held In Reset
Booting
Alive
Warning
Faulted
Timed Out
```

A missing heartbeat should not immediately cause a reset unless the card is expected to be active.

## RTC and NTP Interaction

The CPU or network service may obtain NTP time during boot.

The supervisor should accept RTC updates only if:

```text
NTP result is valid
Time passes sanity checks
Update is authorized by boot policy
Time is newer than firmware build baseline
```

The supervisor should record whether time is:

```text
Unknown
RTC Only
NTP Synchronized
Manually Set
```

## Golden Recovery

The supervisor should support recovery modes:

```text
Golden ROM Boot
Golden FPGA Bitstream
CPU Held In Reset
Fabric Minimal Mode
Network Recovery Boot
UART Monitor Recovery
```

A failed firmware or FPGA update must not permanently brick the system.

## Diagnostic Interface

The supervisor should expose a diagnostic console.

Candidate interfaces:

```text
UART header
USB serial
Management bus command interface
Fabric mailbox endpoint after fabric startup
```

Minimum diagnostic commands:

```text
STATUS
RESET CPU
RESET FABRIC
RESET SLOT <n>
SHOW FAULTS
CLEAR FAULTS
SHOW RTC
SET RTC
SHOW POWER
SHOW HEARTBEATS
ENTER RECOVERY
```

## Integration With Boot Sequence

Supervisor startup occurs before CPU reset release.

Recommended sequence:

```text
Power stable
Supervisor boots
Power rails verified
Recovery inputs read
FPGA mode selected
Fabric reset controlled
CPU reset held
Fabric ready checked if required
CPU reset released
Boot monitored by heartbeat
```

See:

```text
docs/boot-sequence-v1.md
docs/clock-reset-spec.md
```

## Diagnostic Requirements

Required test points:

```text
Supervisor Clock
Supervisor Reset
RTC I2C/SPI Lines
Power Good Inputs
CPU Reset Output
Fabric Reset Output
Slot Reset Outputs
Watchdog Output
Recovery Input
Fault Log Write Indicator
Management Bus Lines
UART TX/RX
```

Required LEDs:

```text
Supervisor Alive
Power Good
Recovery Mode
CPU Reset Asserted
Fabric Reset Asserted
Fault Logged
Management Activity
```

See:

```text
docs/diagnostic-access-v1.md
```

## Security and Authority

The supervisor is the root management authority.

It may:

- Hold cards in reset.
- Select recovery paths.
- Record faults.
- Clear or preserve configuration.
- Authorize RTC updates.

It should not silently perform normal data-plane work.

The supervisor should not grant unrestricted DMA or accelerator access. Those remain governed by the fabric and capability model.

## Relationship to Fabric Controller

The ULX3S/ECP5 fabric controller owns main fabric services.

The supervisor owns management and recovery.

The supervisor may reset or configure the fabric controller, but it should not replace the fabric controller during normal operation.

See:

```text
docs/ulx3s-backplane-controller.md
docs/fpga-fabric.md
```

## Initial Rev A Supervisor Recommendation

The first supervisor card should include:

```text
MCU
RTC
FRAM or EEPROM fault log
Reset-control outputs
Power-good inputs
Recovery DIP/button inputs
UART/USB diagnostic console
Management bus interface
Status LEDs
Test points
```

## Open Questions

- Final MCU selection.
- RTC part selection.
- Fault-log storage technology.
- Management-bus electrical standard.
- Whether supervisor is a separate card or integrated into the backplane for Rev A.
- Number of independently resettable slots.
- Whether the supervisor directly loads FPGA bitstreams or only selects boot mode.
- Whether NTP updates are pushed by CPU firmware or pulled through a supervisor network path.
- Exact fault-log binary format.

## Design Principle

The supervisor is the system's service processor.

It exists so ATARIX can fail visibly, recover deliberately, and retain enough information to diagnose what happened.

<!-- AEMS-MIGRATED-SOURCE: docs/testing-strategy.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/testing-strategy.md -->
### Integrated source: `docs/testing-strategy.md`

# ATARIX Testing Strategy

## Status

Draft project testing specification.

This document defines the testing philosophy, test-suite structure, compatibility goals, and abstraction requirements for ATARIX.

## Purpose

ATARIX is a modular hardware, firmware, FPGA, and operating-system research project. It needs a testing strategy that can survive future CPU cards, fabric revisions, memory-service changes, accelerator cards, and protocol revisions.

Tests should validate behavior, not just one implementation.

## Influences

The `dlworrell/65x02` repository provides a useful testing model for CPU reimplementation work:

- Each test executes only a single instruction.
- Each test provides full processor and memory state before and after execution.
- Tests are generated in large volume.
- Test methodology is documented.
- Test data formats are manually versioned for future breaking changes.

ATARIX should borrow these principles and apply them more broadly across CPU, firmware, FPGA fabric, DMA, mailbox, discovery, supervisor, security, and system tests.

## Design Principles

1. Test behavior, not implementation details.
2. Keep test formats versioned.
3. Make tests reproducible.
4. Preserve before-and-after state where practical.
5. Prefer small targeted tests over vague integration tests.
6. Support large randomized test sets.
7. Make failures diagnosable.
8. Keep tests abstract enough to support future CPU cards and fabric revisions.
9. Avoid hard-coding W65C816 assumptions into tests unless the test is explicitly W65C816-specific.
10. Treat failing tests as engineering data.

## Abstraction Rule

Tests should target stable architectural interfaces.

Examples of stable interfaces:

```text
Mailbox protocol
DMA descriptors
Discovery records
Capability records
Register semantics
Supervisor commands
Boot image format
Memory object model
```

Tests should avoid depending on temporary implementation details unless explicitly marked as implementation-specific.

## Test Categories

### CPU Instruction Tests

CPU instruction tests should follow the 65x02-style model where practical.

Each test should define:

```text
Initial CPU state
Initial memory state
Instruction or operation
Expected CPU state
Expected memory state
Expected cycles or timing, if applicable
Expected exceptions or faults, if applicable
```

W65C816 tests may be CPU-specific.

Future CPU cards should have their own CPU-specific tests while still sharing system-level tests.

### Firmware Tests

Firmware tests should cover:

```text
Reset vector behavior
ROM monitor commands
Memory tests
Discovery parsing
Mailbox commands
DMA submission
Netboot stages
Recovery paths
Error handling
```

### FPGA Fabric Tests

Fabric tests should cover:

```text
Register reads and writes
Mailbox routing
DMA descriptor parsing
Interrupt routing
Discovery response generation
Capability lookup assistance
Window mapping
Fault reporting
Counter behavior
Reset behavior
```

### Protocol Tests

Protocol tests should validate binary structures against the data model.

Required checks:

```text
Little-endian encoding
Field widths
Reserved fields zeroed
Version handling
Unknown optional fields ignored
Unknown mandatory fields rejected
Length and bounds checking
```

Relevant protocols:

```text
Discovery ROM format
Mailbox protocol
DMA descriptors
Capability records
Boot image headers
Supervisor fault logs
```

### Security Tests

Security tests should verify:

```text
Denied access without capability
DMA rejection without valid grants
Capability expiration
Capability revocation
Ring policy enforcement
Device quarantine
Supervisor recovery after violation
Audit event generation
```

### Supervisor Tests

Supervisor tests should verify:

```text
RTC read/write
NTP correction policy
Watchdog escalation
Reset sequencing
Fault logging
Slot quarantine
Recovery mode
Power-good handling
Anomaly scoring
```

### Hardware Bring-Up Tests

Hardware tests should verify:

```text
Power rails
Clock distribution
Reset lines
Bus signal integrity
JTAG/SWD access
UART console
Diagnostic LEDs
Test-point availability
Logic analyzer headers
```

### Integration Tests

Integration tests should combine subsystems carefully.

Examples:

```text
Boot to monitor
Discovery enumeration
Mailbox ping
DMA loopback
Netboot image fetch
Supervisor watchdog recovery
Memory object window mapping
Accelerator command submission
```

## Test Format Versioning

All machine-readable test formats must include:

```text
Format name
Format major version
Format minor version
Target architecture or subsystem
Required feature flags
```

Major version changes may break compatibility.

Minor version changes should be backward-compatible.

## Test Vector Structure

A generic test vector should include:

```text
Test ID
Description
Target subsystem
Required capabilities
Initial state
Operation
Expected final state
Expected faults
Expected logs
Timing expectations, if any
Format version
```

## Randomized Testing

ATARIX should support randomly generated tests where practical.

Examples:

```text
CPU instruction tests
DMA descriptor fuzzing
Mailbox payload fuzzing
Discovery TLV fuzzing
Capability table fuzzing
Register access order tests
```

Randomized tests must record their seed so failures can be reproduced.

## Golden Reference Testing

Where possible, ATARIX should compare behavior against a known-good reference.

Examples:

```text
CPU emulator
Protocol parser reference implementation
Fabric simulation model
Supervisor command simulator
DMA descriptor validator
```

## CI Integration

The repository should eventually run tests through GitHub Actions.

Initial CI targets:

```text
Markdown link checks
Protocol format linting
Schema validation
Unit tests for tools
Firmware build smoke tests
HDL linting
Simulation smoke tests
```

Later CI targets:

```text
Verilator or equivalent simulation
CPU emulator test runs
DMA simulation
Mailbox simulation
Discovery parser tests
Capability validation tests
```

## Hardware-in-the-Loop Testing

Hardware-in-the-loop tests should be added once prototype boards exist.

They should be able to run:

```text
Power-on smoke test
UART monitor test
Supervisor status test
Mailbox loopback
DMA loopback
Reset recovery
Clock scaling
Fault injection
```

## Directory Structure

Recommended structure:

```text
tests/
  README.md
  cpu/
  firmware/
  fabric/
  protocols/
  security/
  supervisor/
  hardware/
  integration/
  vectors/
  schemas/
  tools/
```

## Required Test Documentation

Each test suite should document:

```text
Purpose
Scope
Assumptions
Required hardware or emulator
How to run
Expected output
Failure interpretation
Format version
```

## Compatibility Rule

Tests should assume that ATARIX may eventually include:

```text
W65C816 CPU cards
Future 32-bit CPU cards
Future 64-bit CPU cards
Multiple fabric-controller revisions
Memory-service cards
Storage-service cards
Accelerators
Different supervisor revisions
```

Only CPU-specific tests should assume a specific CPU.

System-level tests should target abstract architectural interfaces.

## Design Principle

Test the architecture as a set of stable contracts.

Implementations may change.

Contracts should remain testable.

<!-- AEMS-MIGRATED-SOURCE: docs/trust-model-v1.md -->
<!-- AEMS-ARCHIVE-REF: docs/archive/atx100-migrated/trust-model-v1.md -->
### Integrated source: `docs/trust-model-v1.md`

# Trust Model v1

Status: Draft v1

## Purpose

This document defines the ATARIX trust model.

Trust answers:

```text
How much confidence should the platform place in this card, provider, service, or resource?
```

Trust does not identify hardware.

Trust does not describe resources.

Trust does not grant capabilities by itself.

## Architectural Chain

```text
Identity
    identifies

Discovery
    describes

Trust
    evaluates

Capabilities
    authorize

Rings
    constrain

Directory
    locates

Transport
    operates
```

## Trust States

Initial trust states:

```text
UNTRUSTED
IDENTIFIED
VALIDATED
TRUSTED
PRIVILEGED
REVOKED
```

## State Definitions

### UNTRUSTED

The platform has insufficient confidence in the entity.

This may occur when:

- identity is missing
- identity validation failed
- discovery validation failed
- policy rejects the entity
- the entity is explicitly blocked

### IDENTIFIED

The entity has a valid identity source.

Identity proves existence only.

IDENTIFIED does not imply operational authority.

### VALIDATED

The entity has passed required validation steps.

Examples:

- Identity EEPROM CRC valid
- Discovery ROM CRC valid
- health checks acceptable
- provisioning records acceptable

### TRUSTED

The entity is trusted by platform policy for normal participation.

TRUSTED does not imply privileged authority.

### PRIVILEGED

The entity is trusted for elevated platform functions.

Examples:

- supervisor-adjacent services
- recovery services
- boot authority
- fabric control
- privileged instrumentation

### REVOKED

The entity is explicitly denied by policy.

Revocation overrides identity, discovery, and prior trust.

## Homebrew Policy

Homebrew cards may be:

```text
Identified
Validated
Enumerated
```

without becoming TRUSTED or PRIVILEGED.

Default homebrew posture:

```text
Identity:      Allowed if valid
Discovery:     Allowed if valid
Validation:    Allowed if safe
Enumeration:   Allowed if policy permits
Capabilities:  Restricted
Supervisor:    Denied
DMA:           Denied unless explicitly authorized
Boot:          Denied unless explicitly authorized
```

## Trust Evaluation Inputs

Trust may consider:

- Identity EEPROM validation state
- Discovery ROM validation state
- board class
- trust class
- provider ID
- provisioning state
- factory test signature
- supervisor policy
- development-mode switch
- ring policy
- future cryptographic signatures
- revocation lists

## Trust and Rings

Trust and rings are separate.

Trust describes platform confidence.

Rings describe authority boundaries.

A trusted service may still be unavailable to Ring 3.

A privileged service may be visible to Ring 0 but controlled only by Ring -2.

## Trust and Capabilities

Capabilities authorize operations.

Trust evaluation determines whether capabilities may be accepted, reduced, or rejected.

A capability from an untrusted or revoked entity shall not be accepted for normal operation.

## Trust Transitions

Typical transition:

```text
UNTRUSTED
    ↓
IDENTIFIED
    ↓
VALIDATED
    ↓
TRUSTED
```

Privileged transition:

```text
TRUSTED
    ↓
PRIVILEGED
```

Revocation:

```text
ANY STATE
    ↓
REVOKED
```

## Recovery Rule

Recovery must not depend on trusted directory services.

Ring -2 supervisor authority must retain enough visibility to diagnose and recover from trust failures.

## Related Documents

- identity-eeprom-v1.md
- discovery-rom-format-v1.md
- ADR-RING-SECURITY-MODEL.md
- ADR-IDENTITY-TRUST-CAPABILITY-SEPARATION.md
- capability-record-format-v1.md
