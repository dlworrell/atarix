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
