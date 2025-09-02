#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
make clean 2>/dev/null || true
make
./HQC_rt_secure --steps 50 --dt 0.005 | tee /tmp/run.log
if grep -qi 'fail' /tmp/run.log; then
  echo "[SMOKE] Found FAIL lines "
  exit 1
else
  echo "[SMOKE] No FAIL lines "
fi
