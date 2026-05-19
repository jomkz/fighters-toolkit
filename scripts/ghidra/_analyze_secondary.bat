@echo off
:: Internal helper: runs DumpOverlayDLL against one secondary project.
:: Usage: _analyze_secondary.bat <LABEL> <PROJ_DIR> <PROJ_NAME> <BINARY_NAME>
::   LABEL       -- display name (e.g. IP.EXE)
::   PROJ_DIR    -- path to the Ghidra project directory
::   PROJ_NAME   -- Ghidra project name (e.g. fa-ip)
::   BINARY_NAME -- binary filename inside the project (e.g. IP.EXE), used for -process

setlocal

set JAVA_HOME=C:\java\jdk-26.0.1
set GHIDRA_HOME=C:\tools\ghidra_12.1_PUBLIC
set FA_PROJECT=%USERPROFILE%\src\fa
set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

set LABEL=%~1
set PROJ_DIR=%~2
set PROJ_NAME=%~3
set BINARY=%~4

if "%LABEL%"=="" (
    echo _analyze_secondary.bat: missing LABEL argument
    exit /b 1
)
if "%PROJ_DIR%"=="" (
    echo _analyze_secondary.bat: missing PROJ_DIR argument
    exit /b 1
)
if "%PROJ_NAME%"=="" (
    echo _analyze_secondary.bat: missing PROJ_NAME argument
    exit /b 1
)
if "%BINARY%"=="" (
    echo _analyze_secondary.bat: missing BINARY argument
    exit /b 1
)

if not exist "%PROJ_DIR%" (
    echo   [%LABEL%] Project dir not found: %PROJ_DIR% -- run --secondary first
    exit /b 0
)

:: DumpOverlayDLL.java writes to Overlay_<LABEL>.txt (label is passed as script arg)
set OUT_FILE=%FA_PROJECT%\output\Overlay_%LABEL%.txt
if exist "%OUT_FILE%" del "%OUT_FILE%"

echo   [%LABEL%] Running DumpOverlayDLL.java (with analysis)...
"%GHIDRA_HOME%\support\analyzeHeadless.bat" "%PROJ_DIR%" "%PROJ_NAME%" ^
    -process "%BINARY%" ^
    -postScript DumpOverlayDLL.java %LABEL% ^
    -scriptPath "%SCRIPT_DIR%" < nul
echo   [%LABEL%] Done.

endlocal
