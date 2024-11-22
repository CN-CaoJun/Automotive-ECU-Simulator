@echo off
pyocd flash --target=STM32H750VBTx  --erase=auto --frequency=1000000 build/rtthread.bin
pause