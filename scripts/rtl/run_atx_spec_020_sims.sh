#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/rtl/atx_spec_020"
LOG_DIR="${BUILD_DIR}/logs"
ARTIFACT_DIR="${BUILD_DIR}/artifacts"
SUMMARY_FILE="${ARTIFACT_DIR}/summary.txt"
SIM_TIMEOUT_SECONDS="${SIM_TIMEOUT_SECONDS:-30}"

mkdir -p "${BUILD_DIR}" "${LOG_DIR}" "${ARTIFACT_DIR}"
cd "${ROOT_DIR}"

printf '%s\n' '============================================================'
printf '%s\n' 'ATX-SPEC-020 RTL simulation runner'
printf 'ROOT_DIR=%s\n' "${ROOT_DIR}"
printf 'BUILD_DIR=%s\n' "${BUILD_DIR}"
printf 'LOG_DIR=%s\n' "${LOG_DIR}"
printf 'ARTIFACT_DIR=%s\n' "${ARTIFACT_DIR}"
printf 'SIM_TIMEOUT_SECONDS=%s\n' "${SIM_TIMEOUT_SECONDS}"
printf '%s\n' '============================================================'

command -v iverilog >/dev/null 2>&1 || {
    printf '%s\n' 'ERROR: iverilog not found on PATH.' >&2
    exit 127
}

command -v vvp >/dev/null 2>&1 || {
    printf '%s\n' 'ERROR: vvp not found on PATH.' >&2
    exit 127
}

if command -v timeout >/dev/null 2>&1; then
    TIMEOUT_CMD=(timeout "${SIM_TIMEOUT_SECONDS}")
else
    TIMEOUT_CMD=()
    printf '%s\n' 'WARN: timeout command not found; simulations will rely on testbench timeouts only.'
fi

printf 'Icarus Verilog: %s\n' "$(iverilog -V | head -n 1 || true)"
printf 'VVP: %s\n' "$(vvp -V | head -n 1 || true)"

ACCEL_BIN="${BUILD_DIR}/atx_spec_020_accelerator_tb"
ACCEL_LOG="${LOG_DIR}/atx_spec_020_accelerator_tb.log"
MODULE_BIN="${BUILD_DIR}/atx_spec_020_modules_tb"
MODULE_LOG="${LOG_DIR}/atx_spec_020_modules_tb.log"

printf '%s\n' '------------------------------------------------------------'
printf '%s\n' 'Step 1: Compile accelerator simulation'
printf '%s\n' 'Command:'
printf '%s\n' "iverilog -g2012 -o ${ACCEL_BIN} rtl/atarix/atx_simd_probe_core.v rtl/atarix/atx_krapivin_stepper.v rtl/atarix/atx_spec_020_accelerator.v rtl/atarix/atx_spec_020_accelerator_ci_tb.v"
iverilog -g2012 \
    -o "${ACCEL_BIN}" \
    rtl/atarix/atx_simd_probe_core.v \
    rtl/atarix/atx_krapivin_stepper.v \
    rtl/atarix/atx_spec_020_accelerator.v \
    rtl/atarix/atx_spec_020_accelerator_ci_tb.v

printf '%s\n' '------------------------------------------------------------'
printf '%s\n' 'Step 2: Run accelerator simulation'
printf '%s\n' 'Command:'
printf '%s\n' "timeout ${SIM_TIMEOUT_SECONDS} vvp ${ACCEL_BIN}"
"${TIMEOUT_CMD[@]}" vvp "${ACCEL_BIN}" | tee "${ACCEL_LOG}"

if ! grep -q 'ATX020 RESULT PASS' "${ACCEL_LOG}"; then
    printf '%s\n' 'ERROR: accelerator simulation did not report ATX020 RESULT PASS.' >&2
    exit 1
fi

if [ -f "atx_spec_020_accelerator_tb.vcd" ]; then
    mv -f "atx_spec_020_accelerator_tb.vcd" "${ARTIFACT_DIR}/"
fi

printf '%s\n' '------------------------------------------------------------'
printf '%s\n' 'Step 3: Compile module validation simulation'
printf '%s\n' 'Command:'
printf '%s\n' "iverilog -g2012 -o ${MODULE_BIN} rtl/atarix/atx_audit_log_window.v rtl/atarix/atx_simd_probe_core.v rtl/atarix/atx_scalar_probe_core.v rtl/atarix/atx_krapivin_stepper.v rtl/atarix/atx_elias_fano_decoder.v rtl/atarix/atx_spec_020_modules_tb.v"
iverilog -g2012 \
    -o "${MODULE_BIN}" \
    rtl/atarix/atx_audit_log_window.v \
    rtl/atarix/atx_simd_probe_core.v \
    rtl/atarix/atx_scalar_probe_core.v \
    rtl/atarix/atx_krapivin_stepper.v \
    rtl/atarix/atx_elias_fano_decoder.v \
    rtl/atarix/atx_spec_020_modules_tb.v

printf '%s\n' '------------------------------------------------------------'
printf '%s\n' 'Step 4: Run module validation simulation'
printf '%s\n' 'Command:'
printf '%s\n' "timeout ${SIM_TIMEOUT_SECONDS} vvp ${MODULE_BIN}"
"${TIMEOUT_CMD[@]}" vvp "${MODULE_BIN}" | tee "${MODULE_LOG}"

if ! grep -q 'ATX020 MODULES RESULT PASS' "${MODULE_LOG}"; then
    printf '%s\n' 'ERROR: module validation simulation did not report ATX020 MODULES RESULT PASS.' >&2
    exit 1
fi

if [ -f "atx_spec_020_modules_tb.vcd" ]; then
    mv -f "atx_spec_020_modules_tb.vcd" "${ARTIFACT_DIR}/"
fi

printf '%s\n' '------------------------------------------------------------'
printf '%s\n' 'Step 5: Generate simulation summary'
{
    printf '%s\n' 'ATX-SPEC-020 RTL Simulation Summary'
    printf '%s\n' '==================================='
    printf '\n'
    printf '%s\n' 'Accelerator simulation: PASS'
    printf '%s\n' 'Module validation simulation: PASS'
    printf '\n'
    printf '%s\n' 'Accelerator PASS lines:'
    grep 'PASS ' "${ACCEL_LOG}" || true
    printf '\n'
    printf '%s\n' 'Module PASS lines:'
    grep 'PASS ' "${MODULE_LOG}" || true
    printf '\n'
    printf '%s\n' 'Scalar-vs-SIMD gain proxy lines:'
    grep -E 'gain proxy|scalar_cycles|Expected gain proxy' "${ACCEL_LOG}" "${MODULE_LOG}" || true
} > "${SUMMARY_FILE}"

cp -f "${ACCEL_LOG}" "${ARTIFACT_DIR}/"
cp -f "${MODULE_LOG}" "${ARTIFACT_DIR}/"

printf '%s\n' '------------------------------------------------------------'
printf '%s\n' 'Step 6: Print summary'
cat "${SUMMARY_FILE}"

printf '%s\n' '============================================================'
printf '%s\n' 'ATX-SPEC-020 RTL simulations completed successfully.'
printf 'Artifacts: %s\n' "${ARTIFACT_DIR}"
printf '%s\n' '============================================================'
