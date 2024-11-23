@echo off
openocd -f interface/cmsis-dap.cfg -f target/stm32h7x.cfg  -c "program  ../Debug/rtthread.bin  verify reset exit 0x08000000"
pause