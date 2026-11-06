#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Negative controls for the CPU card test bench. Each run injects one defect
# that a real implementation could have; every one must make the bench FAIL.
# Usage: sim/cpu-card/negative_control.sh <atarix-cpu-card> <test ROM> <liar ROM>

set -euo pipefail
card="${1:?atarix-cpu-card}"
rom="${2:?test ROM}"
liar="${3:?liar ROM}"

expect_fail() {
    local label="$1"
    shift
    local log status
    log="$(mktemp)"
    "$@" > "${log}" 2>&1
    status=$?
    if [ "${status}" -eq 0 ]; then
        echo "negative control FAILED: ${label} was accepted" >&2
        cat "${log}" >&2
        rm -f "${log}"
        exit 1
    fi
    rm -f "${log}"
    if [ "${status}" -ne 1 ]; then
        echo "negative control FAILED: ${label} exited ${status}, expected test failure (1)" >&2
        exit 1
    fi
    echo "detected: ${label}"
}

set +e
expect_fail "Northbridge skips the capability check" "${card}" --rom "${rom}" --inject skip-capability
expect_fail "Northbridge skips the ring check" "${card}" --rom "${rom}" --inject skip-ring
expect_fail "Northbridge accepts a bad CRC" "${card}" --rom "${rom}" --inject accept-bad-crc
expect_fail "Northbridge corrupts the PONG payload" "${card}" --rom "${rom}" --inject corrupt-pong
expect_fail "firmware claims PASS without running the tests" "${card}" --rom "${liar}"
expect_fail "Vector Pull Rewrite does not rewrite" "${card}" --rom "${rom}" --inject no-vector-rewrite
expect_fail "interrupt mask is ignored" "${card}" --rom "${rom}" --inject ignore-mask
expect_fail "mailbox IRQ dispatched at level 7 (pulls RESET)" "${card}" --rom "${rom}" --inject mailbox-level7
expect_fail "supervisor gateway executes a CPU reset request" "${card}" --rom "${rom}" --inject supervisor-accepts-reset
expect_fail "Northbridge delivers to endpoints that are not ACTIVE" "${card}" --rom "${rom}" --inject skip-lifecycle
expect_fail "Northbridge overwrites an unacknowledged reply" "${card}" --rom "${rom}" --inject skip-resource
expect_fail "directory lists supervisor and fabric endpoints to the kernel" "${card}" --rom "${rom}" --inject list-platform-rings
expect_fail "CPU released from reset before enumeration and binding" "${card}" --rom "${rom}" --inject release-before-enumeration
expect_fail "capabilities granted from discovery instead of policy" "${card}" --rom "${rom}" --inject grant-by-discovery
expect_fail "cards trusted without identity validation" "${card}" --rom "${rom}" --inject skip-identity-validation
expect_fail "GPU service rotates the wrong way" "${card}" --rom "${rom}" --inject gpu-wrong-math
expect_fail "GPU result 3 LSB off (outside tolerance)" "${card}" --rom "${rom}" --inject gpu-off-by-3
set -e
echo "negative controls passed: all seventeen defects were detected"
