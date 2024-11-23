@echo off
@REM pyocd flash --target=STM32H750VBTx --erase=auto --frequency=1000000 Debug/rtthread.bin
@REM pyocd reset
@REM echo Starting Debuging
pyocd  gdbserver -t STM32H750VBTx 
pause