<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
-->

# Vendored: Clock Signal (CLK) CPU cores

Third-party code. Not ATARIX-authored. MIT licensed: see [`LICENCE`](LICENCE)
(Copyright (c) 2015 Thomas Harte). Files keep their upstream headers.

| Field | Value |
|---|---|
| Upstream | https://github.com/dlworrell/CLK (fork of https://github.com/TomHarte/CLK) |
| Pinned commit | `0f49993b4d0992ebba1fad5032d96637f3a84d1f` |
| Language | C++20 |
| Purpose | Cycle-accurate W65C816 core for the emulated ATARIX CPU card, and 6502/65C02 cores for test-harness self-validation |

## Contents

Only the include closure of `Processors/6502Esque/6502Selector.hpp` plus its
compiled storage units is vendored, at the upstream relative paths:

```text
ClockReceiver/ClockReceiver.hpp
ClockReceiver/ForceInline.hpp
Numeric/Carry.hpp
Numeric/RegisterSizes.hpp
Processors/6502/6502.hpp
Processors/6502/Implementation/6502Implementation.hpp
Processors/6502/Implementation/6502Storage.cpp
Processors/6502/Implementation/6502Storage.hpp
Processors/6502Esque/6502Esque.hpp
Processors/6502Esque/6502Selector.hpp
Processors/6502Esque/Implementation/LazyFlags.hpp
Processors/65816/65816.hpp
Processors/65816/Implementation/65816Base.cpp
Processors/65816/Implementation/65816Implementation.hpp
Processors/65816/Implementation/65816Storage.cpp
Processors/65816/Implementation/65816Storage.hpp
```

## Rules

- Do not edit these files. Fix upstream in `dlworrell/CLK`, then re-vendor at
  the new pinned commit and update this table.
- Re-vendor with `scripts/sim/vendor_clk.sh <CLK checkout> <commit>`, which
  recomputes the include closure and fails if the set of files changes.
- The ATARIX simulator treats this core as a model of the W65C816, not as
  authority. CPU-visible behavior is checked against independent vectors
  (`scripts/sim/fetch_cpu_vectors.sh`), not against CLK itself.
- AES-SEC-001: the AEMS scanner (strict, dangerous primitives included)
  reports no findings in this directory at the pinned commit (checked
  2026-11-06), so it stays inside the repository scan. If a future re-vendor
  introduces findings, decide explicitly between recording dispositions and a
  declared third-party exclusion; ATARIX cannot review upstream code as if it
  were its own.
