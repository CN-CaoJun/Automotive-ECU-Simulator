@echo off
set OPENOCD_PATH=_tools\OpenOCD
set FIRMWARE_ELF=build/rtthread.bin

echo ==============================
echo Starting OpenOCD server
echo ==============================

%OPENOCD_PATH%\bin\openocd.exe -f interface\cmsis-dap.cfg -f target\stm32f4x.cfg -c "program  %FIRMWARE_ELF%  verify reset exit 0x08000000"

timeout /t 1

echo ==============================
echo Stopping OpenOCD server
echo Flashing completed
echo ==============================

