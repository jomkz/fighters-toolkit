@echo off
:: Internal helper called by import_overlays.bat.
:: Usage: _import_one.bat FMT OVERLAY_ROOT PROJECTS_ROOT GHIDRA_HOME SCRIPT_DIR
::
:: Patches PL->PE in copies, then imports into a Ghidra project.

setlocal enabledelayedexpansion

set FMT=%~1
set OVERLAY_ROOT=%~2
set PROJECTS_ROOT=%~3
set GHIDRA_HOME=%~4
set SCRIPT_DIR=%~5

set SRC_DIR=%OVERLAY_ROOT%\%FMT%
set PE_DIR=%OVERLAY_ROOT%\%FMT%_pe
set PROJ_DIR=%PROJECTS_ROOT%\%FMT%
set PROJ_NAME=fa-%FMT%

if not exist "%SRC_DIR%" (
    echo   [%FMT%] Source dir not found: %SRC_DIR%
    exit /b 0
)

if not exist "%PE_DIR%" mkdir "%PE_DIR%"
set COUNT=0
for %%F in ("%SRC_DIR%\*") do (
    set "IN=%%F"
    set "OUT=%PE_DIR%\%%~nxF"
    powershell -NoProfile -Command "$b=[System.IO.File]::ReadAllBytes('!IN!');$off=[BitConverter]::ToInt32($b,0x3C);if($b[$off] -eq 0x50 -and $b[$off+1] -eq 0x4C){$b[$off+1]=0x45};[System.IO.File]::WriteAllBytes('!OUT!',$b)"
    set /A COUNT+=1
)
echo   [%FMT%] Patched %COUNT% files to PE signature

if not exist "%PROJ_DIR%" mkdir "%PROJ_DIR%"
echo   [%FMT%] Importing into Ghidra project %PROJ_NAME%...
"%GHIDRA_HOME%\support\analyzeHeadless.bat" "%PROJ_DIR%" %PROJ_NAME% -import "%PE_DIR%" -overwrite -scriptPath "%SCRIPT_DIR%"

echo   [%FMT%] Done.
endlocal
