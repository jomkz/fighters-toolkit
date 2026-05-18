@echo off
:: Imports all extracted overlay DLLs and secondary game binaries into
:: per-format Ghidra projects under %FA_PROJECT%\overlay_projects\.
::
:: Run extract_overlays.bat first to populate the overlay directories.
::
:: Note on Phar Lap PE format:
::   FA overlay DLLs use signature PL\0\0 instead of the standard PE\0\0.
::   Ghidra 12.1 PE loader rejects these. This script patches byte 1 at the
::   PE header offset (read from MZ stub at 0x3C) from 0x4C to 0x45.
::   The patched copies live at %FA_PROJECT%\overlays\{fmt}_pe\.
::
:: Usage:  scripts\ghidra\import_overlays.bat [FORMAT]
::   FORMAT (optional): BI, CAM, MC, HUD, LAY, FNT, MUS, secondary, or ALL (default)

setlocal

set JAVA_HOME=C:\java\jdk-26.0.1
set GHIDRA_HOME=C:\tools\ghidra_12.1_PUBLIC
set FA_PROJECT=%USERPROFILE%\src\fa
set OVERLAY_ROOT=%FA_PROJECT%\overlays
set PROJECTS_ROOT=%FA_PROJECT%\overlay_projects
set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

set TARGET=%1
if "%TARGET%"=="" set TARGET=ALL

echo ============================================================
echo  FA overlay DLL Ghidra import
echo  Projects: %PROJECTS_ROOT%
echo  Target  : %TARGET%
echo ============================================================
echo.

if not exist "%PROJECTS_ROOT%" mkdir "%PROJECTS_ROOT%"

if /I "%TARGET%"=="ALL" goto :all
call "%~dp0_import_one.bat" %TARGET% "%OVERLAY_ROOT%" "%PROJECTS_ROOT%" "%GHIDRA_HOME%" "%SCRIPT_DIR%"
goto :secondary

:all
for %%F in (BI CAM MC HUD LAY FNT MUS) do (
    echo.
    echo --- %%F ---
    call "%~dp0_import_one.bat" %%F "%OVERLAY_ROOT%" "%PROJECTS_ROOT%" "%GHIDRA_HOME%" "%SCRIPT_DIR%"
)

:secondary
set SEC_DIR=%OVERLAY_ROOT%\secondary
if exist "%SEC_DIR%" (
    echo.
    echo --- secondary ---
    if not exist "%PROJECTS_ROOT%\secondary" mkdir "%PROJECTS_ROOT%\secondary"
    "%GHIDRA_HOME%\support\analyzeHeadless.bat" "%PROJECTS_ROOT%\secondary" fa-secondary -import "%SEC_DIR%" -overwrite -scriptPath "%SCRIPT_DIR%"
    echo   [secondary] Done.
)

echo.
echo ============================================================
echo  Import complete.  Projects: %PROJECTS_ROOT%
echo ============================================================
endlocal
