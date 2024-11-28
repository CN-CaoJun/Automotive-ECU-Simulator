@echo off
openocd -f interface/cmsis-dap.cfg -f target/stm32h7x_512K.cfg  -c "program  ../build/Simulated-GW-ECU.bin  verify reset exit 0x08000000"
pause