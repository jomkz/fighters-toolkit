@echo off
:: Internal helper: runs DumpOverlayDLL.java against one overlay project.
:: Usage: _analyze_overlay.bat FMT
::   FMT: BI, CAM, MC, HUD, LAY, FNT, MUS

setlocal

set JAVA_HOME=C:\java\jdk-26.0.1
set GHIDRA_HOME=C:\tools\ghidra_12.1_PUBLIC
set FA_PROJECT=%USERPROFILE%\src\fa
set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

set FMT=%~1
if "%FMT%"=="" (
    echo _analyze_overlay.bat: missing FMT argument
    exit /b 1
)

set PROJ_DIR=%FA_PROJECT%\overlay_projects\%FMT%
set PROJ_NAME=fa-%FMT%

if not exist "%PROJ_DIR%" (
    echo   [%FMT%] Project dir not found: %PROJ_DIR% -- run --import first
    exit /b 0
)

:: Clear output file so each run starts fresh (DumpOverlayDLL appends per-DLL)
if exist "%FA_PROJECT%\output\Overlay_%FMT%.txt" del "%FA_PROJECT%\output\Overlay_%FMT%.txt"

echo   [%FMT%] Running DumpOverlayDLL.java (with analysis)...
"%GHIDRA_HOME%\support\analyzeHeadless.bat" "%PROJ_DIR%" "%PROJ_NAME%/%FMT%_pe" ^
    -process "*.%FMT%" ^
    -postScript DumpOverlayDLL.java %FMT% ^
    -scriptPath "%SCRIPT_DIR%" < nul
echo   [%FMT%] Done. Output: %FA_PROJECT%\output\Overlay_%FMT%.txt

endlocal
