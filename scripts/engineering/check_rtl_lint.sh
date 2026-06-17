#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
LOG_FILE="${ROOT_DIR}/build/engineering-gate/artifacts/verilator-lint.log"

mkdir -p "$(dirname "${LOG_FILE}")"
cd "${ROOT_DIR}"

if ! command -v verilator >/dev/null 2>&1; then
    echo "WARN verilator not found; skipping RTL lint" | tee "${LOG_FILE}"
    exit 0
fi

set +e
verilator --lint-only --Wall \
    rtl/atarix/atx_audit_log_window.v \
    rtl/atarix/atx_simd_probe_core.v \
    rtl/atarix/atx_scalar_probe_core.v \
    rtl/atarix/atx_krapivin_stepper.v \
    rtl/atarix/atx_elias_fano_decoder.v \
    rtl/atarix/atx_northbridge_bus_shim.v \
    rtl/atarix/atx_spec_020_accelerator.v \
    rtl/atarix/atx_spec_020_system_wrapper.v \
    > "${LOG_FILE}" 2>&1
status=$?
set -e

if [ "${status}" -ne 0 ]; then
    echo "FAIL Verilator RTL lint. Diagnostics follow:"
    cat "${LOG_FILE}"
    exit "${status}"
fi

echo "PASS Verilator RTL lint" | tee -a "${LOG_FILE}"
