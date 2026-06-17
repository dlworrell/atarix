#!/usr/bin/env bash
set -euo pipefail

bash scripts/engineering/run_gate.sh
bash scripts/engineering/check_aems_docs.sh
