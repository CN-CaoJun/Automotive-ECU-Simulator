@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0..\flash-windows.ps1"
exit /b %errorlevel%
