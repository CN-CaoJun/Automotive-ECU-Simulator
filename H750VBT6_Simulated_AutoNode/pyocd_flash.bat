@echo off
echo Starting Reset
pyocd reset --target=STM32H750VBTx
echo Starting Flash
pyocd flash --target=STM32H750VBTx  --erase=auto --frequency=1000000 build/Simulated-GW-ECU.elf
@REM pyocd flash --target=STM32H750VBTx  --erase=auto --frequency=1000000 build/Fire_H7.axf
echo Ending
pause