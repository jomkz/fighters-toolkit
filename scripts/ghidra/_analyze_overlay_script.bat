@echo off
:: Internal helper: runs a named analysis script against one overlay project.
:: Usage: _analyze_overlay_script.bat FMT ScriptName.java

setlocal

set JAVA_HOME=C:\java\jdk-26.0.1
set GHIDRA_HOME=C:\tools\ghidra_12.1_PUBLIC
set FA_PROJECT=%USERPROFILE%\src\fa
set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

set FMT=%~1
set SCRIPT=%~2

if "%FMT%"=="" (
    echo _analyze_overlay_script.bat: missing FMT argument
    exit /b 1
)
if "%SCRIPT%"=="" (
    echo _analyze_overlay_script.bat: missing SCRIPT argument
    exit /b 1
)

set PROJ_DIR=%FA_PROJECT%\overlay_projects\%FMT%
set PROJ_NAME=fa-%FMT%

if not exist "%PROJ_DIR%" (
    echo   [%FMT%] Project dir not found: %PROJ_DIR% -- run --import first
    exit /b 0
)

:: Derive output filename from script name (e.g. AnalyzeCAMDLL.java -> AnalyzeCAMDLL.txt)
set SCRIPT_BASE=%SCRIPT:.java=%
if exist "%FA_PROJECT%\output\%SCRIPT_BASE%.txt" del "%FA_PROJECT%\output\%SCRIPT_BASE%.txt"

echo   [%FMT%] Running %SCRIPT% (with analysis)...
"%GHIDRA_HOME%\support\analyzeHeadless.bat" "%PROJ_DIR%" "%PROJ_NAME%/%FMT%_pe" ^
    -process "*.%FMT%" ^
    -postScript %SCRIPT% ^
    -scriptPath "%SCRIPT_DIR%" < nul
echo   [%FMT%] Done.

endlocal
