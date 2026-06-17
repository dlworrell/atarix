#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/engineering-gate"
ARTIFACT_DIR="${BUILD_DIR}/artifacts"
DOC_GRAPH_FILE="${ARTIFACT_DIR}/document-graph.dot"

mkdir -p "${ARTIFACT_DIR}"
cd "${ROOT_DIR}"

printf '%s\n' 'AEMS documentation gate'
printf '%s\n' 'Command: python3 tools/aems/aems.py check-docs'
python3 tools/aems/aems.py check-docs

printf 'Command: python3 tools/aems/aems.py graph > %s\n' "${DOC_GRAPH_FILE}"
python3 tools/aems/aems.py graph > "${DOC_GRAPH_FILE}"

printf 'AEMS document graph artifact: %s\n' "${DOC_GRAPH_FILE}"
printf '%s\n' 'AEMS documentation gate result: PASS'
