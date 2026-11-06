#!/usr/bin/env bash
# SPDX-FileCopyrightText: Donovan Worrell
# SPDX-License-Identifier: Apache-2.0
#
# Cross-compile the display card GPU core (src/firmware/display-card) for the
# RP2350's Cortex-M33 with its single-precision FPU, and run it on QEMU
# mps2-an505 (same Armv8-M Mainline core and FPv5-SP FPU) against the
# independently computed vectors.
#
# Checks:
#   1. the core builds for cortex-m33 / hard float / fpv5-sp-d16 with -Werror;
#   2. the core object contains no double-precision arithmetic (no __aeabi_d*
#      soft-float calls and no .f64 instructions): on the RP2350 that would be
#      slow software emulation and means the core is not single-precision;
#   3. every vector case passes on the emulated M33.
# Negative controls (each must be detected, or the script fails):
#   - the double-precision detector, run on code that does use double;
#   - the M33 test built with one expected value 3 LSB off.
# QEMU is not an RP2350 (no RP2350 peripherals, boot ROM or timing); this
# checks the instruction set, FPU arithmetic and code, not the board.

set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
out="${root_dir}/build/firmware/display-card-m33"
mkdir -p "${out}"
cd "${root_dir}"

for tool in arm-none-eabi-gcc arm-none-eabi-objdump qemu-system-arm python3; do
    command -v "${tool}" >/dev/null 2>&1 || { echo "missing tool: ${tool}" >&2; exit 127; }
done

cpu_flags=(-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16)
cflags=(-std=c11 -O2 -g -Wall -Wextra -Werror -Wdouble-promotion -ffunction-sections -fdata-sections
        -Isrc/firmware/display-card -I"${out}" -isystem third_party/cglm)

python3 -I sim/cpu-card/firmware/gen_gpu_vectors.py --c "${out}/gpu_vectors.h"

arm-none-eabi-gcc "${cpu_flags[@]}" "${cflags[@]}" -c src/firmware/display-card/gpu_core.c -o "${out}/gpu_core.o"

# 2. Single precision only.
uses_double() {
    arm-none-eabi-objdump -d -r "$1" | grep -qE '__aeabi_d|__(add|sub|mul|div)df3|__extendsfdf2|__truncdfsf2|\.f64'
}
arm-none-eabi-objdump -d -r "${out}/gpu_core.o" > "${out}/gpu_core.dis"
if uses_double "${out}/gpu_core.o"; then
    echo "FAIL: double-precision arithmetic in the GPU core" >&2
    exit 1
fi
printf 'double f(double a, float b) { return a * (double)b + 1.0; }\n' > "${out}/double_probe.c"
arm-none-eabi-gcc "${cpu_flags[@]}" -std=c11 -O2 -c "${out}/double_probe.c" -o "${out}/double_probe.o"
if ! uses_double "${out}/double_probe.o"; then
    echo "FAIL: negative control: the double-precision detector missed real double arithmetic" >&2
    exit 1
fi
echo "ok   negative control: double-precision detector flags code that uses double"
echo "ok   GPU core is single-precision only ($(grep -cE '\bv[a-z]+\.f32' "${out}/gpu_core.dis") FPU f32 instructions, 0 double)"

build_test() {
    arm-none-eabi-gcc "${cpu_flags[@]}" "${cflags[@]}" "$@" --specs=rdimon.specs -nostartfiles \
        -T src/firmware/display-card/m33/an505.ld -Wl,--gc-sections \
        src/firmware/display-card/m33/startup.c src/firmware/display-card/m33/gpu_core_test.c \
        "${out}/gpu_core.o" -Wl,--start-group -lrdimon -lc -lm -lgcc -Wl,--end-group \
        "$(arm-none-eabi-gcc "${cpu_flags[@]}" -print-file-name=rdimon-crt0.o)"
}
run_m33() {
    timeout 60 qemu-system-arm -M mps2-an505 -cpu cortex-m33 -nographic -monitor none -serial none \
        -semihosting-config enable=on,target=native -kernel "$1" > "$2" 2>&1
}

# 3. Run on the emulated Cortex-M33.
build_test -o "${out}/gpu_core_test.elf"
set +e
run_m33 "${out}/gpu_core_test.elf" "${out}/run.log"
status=$?
set -e
cat "${out}/run.log"
if [ "${status}" -ne 0 ] || ! grep -q "M33 GPU CORE: .* PASS" "${out}/run.log"; then
    echo "FAIL: GPU core on Cortex-M33 (exit ${status})" >&2
    exit 1
fi

# Negative control: the same run with one expected value 3 LSB off must fail.
build_test -DNEGATIVE_CONTROL -o "${out}/gpu_core_test_negative.elf"
set +e
run_m33 "${out}/gpu_core_test_negative.elf" "${out}/run-negative.log"
status=$?
set -e
if [ "${status}" -eq 0 ] || ! grep -q "FAIL g2_general_expected_off_by_3" "${out}/run-negative.log"; then
    echo "FAIL: negative control: a 3 LSB error was accepted on the M33 (exit ${status})" >&2
    cat "${out}/run-negative.log" >&2
    exit 1
fi
echo "ok   negative control: a 3 LSB error is rejected on the M33 (exit ${status})"
