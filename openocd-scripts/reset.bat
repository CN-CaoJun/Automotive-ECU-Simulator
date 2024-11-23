@echo off
openocd -f interface/cmsis-dap.cfg -f target/stm32h7x.cfg  -c "reset exit"
pause