@echo off
openocd -c "bindto 0.0.0.0" -f interface/cmsis-dap.cfg -f target/stm32h7x.cfg
pause