@echo off
@REM openocd -f interface/cmsis-dap.cfg -f target/stm32h7x.cfg  -c "program  ../build/Fire-H7.hex  verify reset exit 0x08000000"
openocd -f interface/cmsis-dap.cfg -f target/stm32h7x.cfg  -c "program  ../build/Simulated-GW-ECU.bin  verify reset exit 0x08000000"
pause