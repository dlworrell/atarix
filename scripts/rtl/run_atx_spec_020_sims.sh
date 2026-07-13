#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/rtl/atx_spec_020"
LOG_DIR="${BUILD_DIR}/logs"
ARTIFACT_DIR="${BUILD_DIR}/artifacts"
SUMMARY_FILE="${ARTIFACT_DIR}/summary.txt"
SIM_TIMEOUT_SECONDS="${SIM_TIMEOUT_SECONDS:-30}"
RANDOM_SEEDS="${RANDOM_SEEDS:-33562662 305419896 1592594996}"

mkdir -p "${BUILD_DIR}" "${LOG_DIR}" "${ARTIFACT_DIR}"
cd "${ROOT_DIR}"

printf '%s\n' '============================================================'
printf '%s\n' 'ATX-SPEC-020 RTL simulation runner'
printf 'ROOT_DIR=%s\n' "${ROOT_DIR}"
printf 'BUILD_DIR=%s\n' "${BUILD_DIR}"
printf 'LOG_DIR=%s\n' "${LOG_DIR}"
printf 'ARTIFACT_DIR=%s\n' "${ARTIFACT_DIR}"
printf 'SIM_TIMEOUT_SECONDS=%s\n' "${SIM_TIMEOUT_SECONDS}"
printf 'RANDOM_SEEDS=%s\n' "${RANDOM_SEEDS}"
printf '%s\n' '============================================================'

command -v iverilog >/dev/null 2>&1 || { printf '%s\n' 'ERROR: iverilog not found on PATH.' >&2; exit 127; }
command -v vvp >/dev/null 2>&1 || { printf '%s\n' 'ERROR: vvp not found on PATH.' >&2; exit 127; }

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
RANDOM_BIN="${BUILD_DIR}/atx_spec_020_random_tb"
RANDOM_COMBINED_LOG="${LOG_DIR}/atx_spec_020_random_tb.log"

printf '%s\n' '------------------------------------------------------------'
printf '%s\n' 'Step 1: Compile accelerator simulation'
iverilog -g2012 -o "${ACCEL_BIN}" \
    rtl/atarix/atx_simd_probe_core.v \
    rtl/atarix/atx_krapivin_stepper.v \
    rtl/atarix/atx_spec_020_accelerator.v \
    rtl/atarix/atx_spec_020_accelerator_ci_tb.v

printf '%s\n' 'Step 2: Run accelerator simulation'
"${TIMEOUT_CMD[@]}" vvp "${ACCEL_BIN}" | tee "${ACCEL_LOG}"
grep -q 'ATX020 RESULT PASS' "${ACCEL_LOG}" || { printf '%s\n' 'ERROR: accelerator simulation failed.' >&2; exit 1; }
[ ! -f atx_spec_020_accelerator_tb.vcd ] || mv -f atx_spec_020_accelerator_tb.vcd "${ARTIFACT_DIR}/"

printf '%s\n' '------------------------------------------------------------'
printf '%s\n' 'Step 3: Compile module validation simulation'
iverilog -g2012 -o "${MODULE_BIN}" \
    rtl/atarix/atx_audit_log_window.v \
    rtl/atarix/atx_simd_probe_core.v \
    rtl/atarix/atx_scalar_probe_core.v \
    rtl/atarix/atx_krapivin_stepper.v \
    rtl/atarix/atx_elias_fano_decoder.v \
    rtl/atarix/atx_spec_020_modules_tb.v

printf '%s\n' 'Step 4: Run module validation simulation'
"${TIMEOUT_CMD[@]}" vvp "${MODULE_BIN}" | tee "${MODULE_LOG}"
grep -q 'ATX020 MODULES RESULT PASS' "${MODULE_LOG}" || { printf '%s\n' 'ERROR: module validation simulation failed.' >&2; exit 1; }
[ ! -f atx_spec_020_modules_tb.vcd ] || mv -f atx_spec_020_modules_tb.vcd "${ARTIFACT_DIR}/"

printf '%s\n' '------------------------------------------------------------'
printf '%s\n' 'Step 5: Compile seeded randomized regression'
iverilog -g2012 -o "${RANDOM_BIN}" \
    rtl/atarix/atx_simd_probe_core.v \
    rtl/atarix/atx_krapivin_stepper.v \
    rtl/atarix/atx_spec_020_accelerator.v \
    rtl/atarix/atx_spec_020_random_tb.v

: > "${RANDOM_COMBINED_LOG}"
printf '%s\n' 'Step 6: Run seeded randomized regression'
for seed in ${RANDOM_SEEDS}; do
    seed_log="${LOG_DIR}/atx_spec_020_random_seed_${seed}.log"
    printf 'Random regression seed=%s\n' "${seed}" | tee -a "${RANDOM_COMBINED_LOG}"
    "${TIMEOUT_CMD[@]}" vvp "${RANDOM_BIN}" +SEED="${seed}" | tee "${seed_log}"
    cat "${seed_log}" >> "${RANDOM_COMBINED_LOG}"
    grep -q 'ATX020 RANDOM RESULT PASS' "${seed_log}" || {
        printf 'ERROR: randomized regression failed for seed %s.\n' "${seed}" >&2
        exit 1
    }
    cp -f "${seed_log}" "${ARTIFACT_DIR}/"
done

printf '%s\n' '------------------------------------------------------------'
printf '%s\n' 'Step 7: Generate simulation summary'
{
    printf '%s\n' 'ATX-SPEC-020 RTL Simulation Summary'
    printf '%s\n' '==================================='
    printf '\nAccelerator simulation: PASS\n'
    printf '%s\n' 'Module validation simulation: PASS'
    printf 'Randomized regression: PASS seeds=%s\n' "${RANDOM_SEEDS}"
    printf '\nAccelerator PASS lines:\n'
    grep 'PASS ' "${ACCEL_LOG}" || true
    printf '\nModule PASS lines:\n'
    grep 'PASS ' "${MODULE_LOG}" || true
    printf '\nRandom regression summaries:\n'
    grep 'ATX020 RANDOM SUMMARY' "${RANDOM_COMBINED_LOG}" || true
    printf '\nScalar-vs-SIMD gain proxy lines:\n'
    grep -E 'gain proxy|scalar_cycles|Expected gain proxy' "${ACCEL_LOG}" "${MODULE_LOG}" || true
} > "${SUMMARY_FILE}"

cp -f "${ACCEL_LOG}" "${ARTIFACT_DIR}/"
cp -f "${MODULE_LOG}" "${ARTIFACT_DIR}/"
cp -f "${RANDOM_COMBINED_LOG}" "${ARTIFACT_DIR}/"

printf '%s\n' 'Step 8: Print summary'
cat "${SUMMARY_FILE}"
printf '%s\n' '============================================================'
printf '%s\n' 'ATX-SPEC-020 RTL simulations completed successfully.'
printf 'Artifacts: %s\n' "${ARTIFACT_DIR}"
printf '%s\n' '============================================================'
