#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/engineering-gate"
ARTIFACT_DIR="${BUILD_DIR}/artifacts"
SUMMARY_FILE="${ARTIFACT_DIR}/summary.txt"

mkdir -p "${BUILD_DIR}" "${ARTIFACT_DIR}"
cd "${ROOT_DIR}"

PASS_COUNT=0
FAIL_COUNT=0
WARN_COUNT=0

log() {
    printf '%s\n' "$*"
}

phase() {
    log '============================================================'
    log "$1"
    log '============================================================'
}

pass() {
    PASS_COUNT=$((PASS_COUNT + 1))
    log "PASS: $1"
}

warn() {
    WARN_COUNT=$((WARN_COUNT + 1))
    log "WARN: $1"
}

fail() {
    FAIL_COUNT=$((FAIL_COUNT + 1))
    log "FAIL: $1"
}

require_file() {
    local path="$1"
    if [ -f "${path}" ]; then
        pass "file exists: ${path}"
    else
        fail "missing required file: ${path}"
    fi
}

phase 'Engineering Gate: environment'
log "ROOT_DIR=${ROOT_DIR}"
log "BUILD_DIR=${BUILD_DIR}"
log "ARTIFACT_DIR=${ARTIFACT_DIR}"
log "SHELL=${SHELL:-unknown}"

phase 'Engineering Gate: repository structure'
require_file 'docs/architecture/ATX-SPEC-015-POSIX-Compatibility-Model.md'
require_file 'docs/architecture/ATX-SPEC-020-Hash-Table-and-Lookup-Acceleration-Model.md'
require_file 'docs/architecture/ATX-SPEC-020-Hardware-Mailbox-Integration-Notes.md'
require_file 'docs/reviews/Architecture-Review-003.md'
require_file 'include/atarix/mailbox.h'
require_file 'include/atarix/index.h'
require_file 'rtl/atarix/README.md'
require_file 'rtl/atarix/ATX-SPEC-020-CI.md'
require_file 'scripts/rtl/run_atx_spec_020_sims.sh'

phase 'Engineering Gate: RTL file inventory'
require_file 'rtl/atarix/atx_northbridge_bus_shim.v'
require_file 'rtl/atarix/atx_spec_020_system_wrapper.v'
require_file 'rtl/atarix/atx_spec_020_accelerator.v'
require_file 'rtl/atarix/atx_simd_probe_core.v'
require_file 'rtl/atarix/atx_scalar_probe_core.v'
require_file 'rtl/atarix/atx_krapivin_stepper.v'
require_file 'rtl/atarix/atx_elias_fano_decoder.v'
require_file 'rtl/atarix/atx_audit_log_window.v'
require_file 'rtl/atarix/atx_spec_020_accelerator_tb.v'
require_file 'rtl/atarix/atx_spec_020_modules_tb.v'

phase 'Engineering Gate: architecture text invariants'
if grep -q 'Lookup acceleration is not authority' docs/architecture/ATX-SPEC-020-Hash-Table-and-Lookup-Acceleration-Model.md; then
    pass 'ATX-SPEC-020 preserves lookup authority invariant'
else
    fail 'ATX-SPEC-020 missing lookup authority invariant'
fi

if grep -q 'Every accelerated lookup must be reconstructable inside a bounded audit window' docs/architecture/ATX-SPEC-020-Hash-Table-and-Lookup-Acceleration-Model.md; then
    pass 'ATX-SPEC-020 preserves audit window invariant'
else
    fail 'ATX-SPEC-020 missing audit window invariant'
fi

if grep -q 'POSIX is a compatibility personality' docs/architecture/ATX-SPEC-015-POSIX-Compatibility-Model.md; then
    pass 'ATX-SPEC-015 preserves POSIX personality invariant'
else
    fail 'ATX-SPEC-015 missing POSIX personality invariant'
fi

phase 'Engineering Gate: C header syntax smoke test'
if command -v gcc >/dev/null 2>&1; then
    C_SMOKE="${BUILD_DIR}/header_smoke.c"
    cat > "${C_SMOKE}" <<'CSMOKE'
#include "include/atarix/mailbox.h"
#include "include/atarix/index.h"
int main(void) {
    atarix_mailbox_header_v1_t header;
    atarix_payload_index_query_v1_t query;
    atarix_payload_index_reply_v1_t reply;
    (void)header;
    (void)query;
    (void)reply;
    return 0;
}
CSMOKE
    if gcc -std=c11 -Wall -Wextra -I. -c "${C_SMOKE}" -o "${BUILD_DIR}/header_smoke.o"; then
        pass 'C header smoke test compiled with gcc'
    else
        fail 'C header smoke test failed with gcc'
    fi
else
    warn 'gcc not found; skipping C header smoke test'
fi

phase 'Engineering Gate: RTL simulation gate'
if command -v iverilog >/dev/null 2>&1 && command -v vvp >/dev/null 2>&1; then
    if bash scripts/rtl/run_atx_spec_020_sims.sh; then
        pass 'ATX-SPEC-020 RTL simulations passed'
        if [ -d build/rtl/atx_spec_020/artifacts ]; then
            cp -R build/rtl/atx_spec_020/artifacts "${ARTIFACT_DIR}/atx_spec_020_rtl"
        fi
    else
        fail 'ATX-SPEC-020 RTL simulations failed'
    fi
else
    warn 'iverilog/vvp not found; skipping RTL simulations'
fi

phase 'Engineering Gate: summary'
{
    printf '%s\n' 'Engineering Gate Summary'
    printf '%s\n' '========================'
    printf 'PASS=%s\n' "${PASS_COUNT}"
    printf 'WARN=%s\n' "${WARN_COUNT}"
    printf 'FAIL=%s\n' "${FAIL_COUNT}"
} > "${SUMMARY_FILE}"

cat "${SUMMARY_FILE}"

if [ "${FAIL_COUNT}" -ne 0 ]; then
    log 'ENGINEERING GATE RESULT: FAIL'
    exit 1
fi

log 'ENGINEERING GATE RESULT: PASS'
