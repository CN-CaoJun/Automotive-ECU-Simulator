#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENOCD="${OPENOCD:-openocd}"

cd "${SCRIPT_DIR}"
"${OPENOCD}" -f interface/cmsis-dap.cfg -f target/stm32h7x_extern.cfg -f flash_boot.cfg
"${OPENOCD}" -f interface/cmsis-dap.cfg -f target/stm32h7x_extern.cfg -f flash_app.cfg
