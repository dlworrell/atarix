---
document: ATX-GPU-SVC-001
title: GPU compute service reached by capability through the mailbox
type: Closure Unit
status: Draft
lifecycle: Draft
related:
  - ATX-SIM-CLOSURE-INDEX
  - ATX-MBX-DENY-001
implements:
  - src/firmware/display-card/gpu_core.c
  - sim/cpu-card/gpu_service.cpp
  - sim/cpu-card/firmware/test_rom.s
verified_by:
  - sim/cpu-card/firmware/gen_gpu_vectors.py
  - scripts/firmware/run_gpu_core_m33.sh
  - sim/cpu-card/negative_control.sh
---
<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
Catalyst-File-ID: urn:uuid:8f8ac81c-3d81-402f-bece-445074abfe55
-->

# ATX-GPU-SVC-001: GPU compute service reached by capability through the mailbox

```yaml
closure_unit:
  id: ATX-GPU-SVC-001
  claim: >-
    The CPU card obtains correct point transforms (rotate about an axis, then
    translate; 16.16 fixed point) from a display card's SERVICE_GPU_COMPUTE
    through the mailbox, only from a GPU it holds a READ capability for, and
    malformed requests are answered with an explicit error status.
  intent:
    authority: draft
    sources:
      - docs/service-id-registry-v1.md (SERVICE_GPU_COMPUTE 0x6000)
      - docs/resource-type-registry-v1.md (RESOURCE_GPU 0x6000)
      - docs/fabric-service-model-v1.md
      - docs/verification/cpu-card-software-testbench.md (GPU compute service)
      - docs/adr/ADR-0016-display-card-gpu-compute-rp2350.md
  realizations:
    public_header: required            # GPU_TRANSFORM_REQUEST/REPLY 0x0040/0x0041 in mailbox.h (proposed)
    service_core: required             # src/firmware/display-card/gpu_core.c (C11, single precision, cglm 58d8c15)
    emulator_service: required         # gpu_service.cpp links the same core
    firmware: required                 # T31-T35 in the CPU card test ROM
    cortex_m33_build: required         # same core for the RP2350's M33 (FPv5-SP), run on QEMU mps2-an505
    rp2350_firmware_image: deferred    # SDK build, startup, mailbox transport, signed image (ADR-0016)
    rtl: deferred
    physical_hardware: deferred
  evidence:
    independent_expected_values: required  # Python double precision (Rodrigues), not cglm: 0 LSB difference
    m33_execution: required            # 6/6 on emulated Cortex-M33, 0 LSB; 103 f32 instructions, no double
    m33_negative_controls: required    # double probe flagged; 3 LSB error rejected
    emulator_tests: required           # T31-T35 pass; host GPU log statuses 0, 0, 2, 1
    capability_denial: required        # T35: second GPU, no capability -> CAPABILITY_DENIED
    negative_controls: required        # wrong rotation and 3 LSB error: both detected at T31
    rtl_frame_equivalence: required    # GPU request frames match in RTL co-simulation
    hardware_capture: deferred
  security_impact: >-
    GPU access is a capability (READ on the GPU's handle); a second GPU without
    a grant is denied. The service validates length and operation and fails
    with an explicit status.
  compatibility_impact: new mailbox message types 0x0040 and 0x0041
  recovery_impact: none
  state: VERIFYING
```

## Open decisions

- Message types `0x0040`/`0x0041` and the payload layout.
- `READ` as the operation a compute request needs.
- The 16.16 format, and whether later operations (matrix upload, batches)
  need larger payloads than the 48-byte TX window carries.
- Processor decided: a dedicated RP2350, Cortex-M33 cores (ADR-0016).
- Signed-image policy so the display card firmware behaves as ROM.
