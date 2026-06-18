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
