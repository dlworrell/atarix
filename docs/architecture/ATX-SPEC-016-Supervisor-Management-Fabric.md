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
