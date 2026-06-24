@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "POWERSHELL_EXE=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"

if not exist "%POWERSHELL_EXE%" (
    echo Cannot find Windows PowerShell: %POWERSHELL_EXE%
    echo Please run dev.ps1 from PowerShell, or repair the Windows PowerShell installation.
    exit /b 1
)

"%POWERSHELL_EXE%" -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%dev.ps1" %*
exit /b %ERRORLEVEL%
