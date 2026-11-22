@echo off
echo Starting Reset
pyocd reset --target=STM32H750VBTx
@REM echo Starting Flash
@REM pyocd flash --target=STM32H750VBTx  --erase=auto --frequency=1000000 build/Simulated-GW-ECU.elf
echo Ending Reset
@REM pause