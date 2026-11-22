#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENOCD="${OPENOCD:-openocd}"

cd "${SCRIPT_DIR}"
"${OPENOCD}" \
    -f interface/cmsis-dap.cfg \
    -f "${SCRIPT_DIR}/target/stm32h7x_extern.cfg" \
    -c 'init; reset halt; source [file join [pwd] qspi_init.tcl]; qspi_prepare; reset run; shutdown'
