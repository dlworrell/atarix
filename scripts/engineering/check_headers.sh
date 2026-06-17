#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/engineering-gate/headers"
LOG_FILE="${ROOT_DIR}/build/engineering-gate/artifacts/header-checks.log"

mkdir -p "${BUILD_DIR}" "$(dirname "${LOG_FILE}")"
cd "${ROOT_DIR}"
: > "${LOG_FILE}"

C_FLAGS="-std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wundef -Werror -Iinclude"
CXX_FLAGS="-std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wundef -Werror -Iinclude"

status=0

mapfile -t HEADERS < <(find include/atarix -type f -name '*.h' | sort)

if [ "${#HEADERS[@]}" -eq 0 ]; then
    echo "FAIL no public headers found under include/atarix" | tee -a "${LOG_FILE}"
    exit 1
fi

echo "Header inventory: ${#HEADERS[@]} public headers" | tee -a "${LOG_FILE}"

for header in "${HEADERS[@]}"; do
    public_name="${header#include/}"
    safe_name="$(printf '%s' "${public_name}" | tr '/.' '__')"

    c_file="${BUILD_DIR}/${safe_name}.c"
    c_obj="${BUILD_DIR}/${safe_name}.c.o"
    cpp_file="${BUILD_DIR}/${safe_name}.cpp"
    cpp_obj="${BUILD_DIR}/${safe_name}.cpp.o"

    printf '#include "%s"\nint main(void) { return 0; }\n' "${public_name}" > "${c_file}"
    printf '#include "%s"\nint main() { return 0; }\n' "${public_name}" > "${cpp_file}"

    echo "C header check: ${public_name}" | tee -a "${LOG_FILE}"
    if command -v gcc >/dev/null 2>&1; then
        if gcc ${C_FLAGS} -c "${c_file}" -o "${c_obj}" >> "${LOG_FILE}" 2>&1; then
            echo "PASS C ${public_name}" | tee -a "${LOG_FILE}"
        else
            echo "FAIL C ${public_name}" | tee -a "${LOG_FILE}"
            status=1
        fi
    else
        echo "WARN gcc not found" | tee -a "${LOG_FILE}"
    fi

    echo "C++ header check: ${public_name}" | tee -a "${LOG_FILE}"
    if command -v g++ >/dev/null 2>&1; then
        if g++ ${CXX_FLAGS} -c "${cpp_file}" -o "${cpp_obj}" >> "${LOG_FILE}" 2>&1; then
            echo "PASS CXX ${public_name}" | tee -a "${LOG_FILE}"
        else
            echo "FAIL CXX ${public_name}" | tee -a "${LOG_FILE}"
            status=1
        fi
    else
        echo "WARN g++ not found" | tee -a "${LOG_FILE}"
    fi
done

exit "${status}"
