#!/usr/bin/env bash
set -euo pipefail

# name of your HW3 folder
HW3_DIR="homework3-http-checker-public"

echo "→ Removing any old client binary…"
rm -f "${HW3_DIR}/client"

echo "→ Building client…
"
make

echo "→ Moving new client into ${HW3_DIR}/"
mv client "${HW3_DIR}/"

echo "→ Entering ${HW3_DIR}/ and running checker…"
cd "${HW3_DIR}"

python3 checker.py --admin ionut_gabriel.mantu:1ef8f46dd0a5 ./client
