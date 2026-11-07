<!--
SPDX-FileCopyrightText: Donovan Worrell
SPDX-License-Identifier: Apache-2.0
-->

# ATARIX machine driver for MAME

This directory holds the MAME machine driver that emulates the ATARIX CPU card as [ATX-SPEC-022](../../../docs/architecture/ATX-SPEC-022-CPU-Card-Platform-Contract.md) defines it (profile EP-A), under [ADR-0016](../../../docs/adr/ADR-0016-cpu-card-emulation-with-mame.md).

The driver is an overlay. It is copied into a pinned MAME release, and only the ATARIX driver is built. There is no MAME fork.

## Licence

The driver source (`src/mame/atarix/atarix.cpp`) is BSD-3-Clause, the convention for MAME's own files, so that it can be offered upstream. Everything else in this directory, and the ROM monitor, is Apache-2.0, as is the rest of ATARIX.

This licence split is the default proposed on PR #132. It is **pending the owner's decision**.

## Status (milestone M0)

| Area | State |
| --- | --- |
| W65C816S at 8 MHz | Done |
| RAM `$000000-$00BFFF` and `$010000-$EFFFFF` | Done |
| 64 KiB boot ROM at `$FF0000`, top 8 KiB mirrored to bank 0 | Done |
| UART console at `$00C000` | Done |
| Board identity (`$00C300`) | Done |
| Fabric control (`$00D000`) with fault recording | Done |
| Unassigned access → `$FF`, fault bit 0, fault address | Done |
| Interrupt controller, timer | M1 |
| Supervisor mailbox, reset cause, watchdog | M2 |
| Mailbox engine, discovery services | M3 |
| Quickload, BIB hand-off | M4 |

## Build and run

You need cc65 for the monitor, and MAME's usual build dependencies (SDL2 and fontconfig among them).

1. Build the ROM monitor:

   ```sh
   src/firmware/monitor/build.sh build/monitor
   ```

2. Fetch the pinned MAME release (the tag and commit are in `.github/workflows/emulator-ep-a.yml`) and overlay the driver:

   ```sh
   git clone --depth 1 --branch mame0289 https://github.com/mamedev/mame build/mame
   src/emulator/mame/overlay.sh build/mame
   ```

3. Build only the ATARIX driver:

   ```sh
   make -C build/mame -j"$(nproc)" SUBTARGET=atarix SOURCES=src/mame/atarix/atarix.cpp
   ```

4. Run it. The console appears in the terminal window:

   ```sh
   build/mame/atarix atarix -cart build/monitor/atarix-monitor.bin
   ```

   Add `-debug` for MAME's debugger, which has breakpoints, watchpoints, trace and memory views.

## Conformance

`conformance/capture_console.lua` records everything written to the UART. `conformance/check_banner.py` then checks the boot banner, which is ATX-SPEC-022 conformance check 1 as far as M0 goes. The reset reason is only checked for presence, because it needs the M2 supervisor mailbox.

CI runs both headless in `.github/workflows/emulator-ep-a.yml`, and keeps the ROM image, its listing and the console transcript as evidence.
