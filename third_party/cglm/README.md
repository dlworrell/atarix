<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
-->

# Vendored: cglm (headers only)

Third-party code. Not ATARIX-authored. MIT licensed: see [`LICENSE`](LICENSE)
(Copyright (c) 2015 Recep Aslantas). Files keep their upstream headers.

| Field | Value |
|---|---|
| Upstream | https://github.com/dlworrell/cglm (fork of https://github.com/recp/cglm) |
| Pinned commit | `58d8c15a124c202188d0094705f778f6129a2612` |
| Language | C99, header-only (single-precision `float`) |
| Purpose | Reference implementation of the simulated GPU compute service (`SERVICE_GPU_COMPUTE`, `0x6000`): the math the display card's own processor would run from its boot ROM |

## Contents

The textual `#include` closure of `cglm/affine.h`, `cglm/mat4.h` and
`cglm/vec3.h`, following every include regardless of platform conditionals
(27 headers), so the copy builds with whichever SIMD path the host selects:
SSE/AVX, NEON, WASM or scalar.

## Rules

- Do not edit these files. Re-vendor with
  `scripts/sim/vendor_cglm.sh <cglm checkout> <commit>` and update this table.
- cglm cannot run on the W65C816: it needs single-precision floating point.
  In ATARIX it belongs to a display or GPU card's own processor (a soft RISC-V
  core with floating point in the ECP5, or an RP2350), reached over the mailbox.
- cglm enables SIMD automatically (SSE on x86); there is no switch to force
  scalar code. The service converts results to 16.16 fixed point, and tests
  compare with a ±2 LSB tolerance, which is far wider than single-precision
  rounding differences between SIMD and scalar paths.
- AES-SEC-001: checked with the AEMS scanner when vendored; see the test-bench
  document for the result.
