@echo off
:: Full overlay pipeline: extract PE DLLs from FA_2.LIB, import into Ghidra,
:: and optionally run analysis scripts or import secondary binaries.
::
:: Usage:
::   run_overlays.bat                    -- extract + import all formats
::   run_overlays.bat --extract          -- extraction step only
::   run_overlays.bat --import           -- import step only (extraction must have run first)
::   run_overlays.bat --import BI        -- import a single format (BI, CAM, MC, HUD, LAY, FNT, MUS)
::   run_overlays.bat --analyze          -- run DumpOverlayDLL against all overlay projects
::   run_overlays.bat --analyze BI       -- run DumpOverlayDLL against a single overlay project
::   run_overlays.bat --analyze-cam      -- run AnalyzeCAMDLL against the CAM overlay project
::   run_overlays.bat --analyze-mc       -- run AnalyzeMCDLL against the MC overlay project
::   run_overlays.bat --secondary            -- import secondary binaries (IP.EXE, WAIL32.DLL, msapi.dll, CD-ROM, serial)
::   run_overlays.bat --secondary IP         -- import a single secondary binary
::   run_overlays.bat --analyze-secondary    -- run DumpOverlayDLL against all secondary projects
::
:: Output projects: %FA_PROJECT%\overlay_projects\{BI,CAM,MC,HUD,LAY,FNT,MUS,secondary}
::                  %FA_PROJECT%\secondary_projects\{ip,wail32,msapi,cdrom,serial}

setlocal

set JAVA_HOME=C:\java\jdk-26.0.1
set GHIDRA_HOME=C:\tools\ghidra_12.1_PUBLIC
set FA_PROJECT=%USERPROFILE%\src\fa
set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

set DO_EXTRACT=1
set DO_IMPORT=1
set DO_ANALYZE=0
set DO_ANALYZE_CAM=0
set DO_ANALYZE_MC=0
set DO_SECONDARY=0
set DO_ANALYZE_SECONDARY=0
set IMPORT_TARGET=ALL
set ANALYZE_TARGET=ALL
set SECONDARY_TARGET=ALL
set SEC_ROOT=%FA_PROJECT%\secondary_projects

if /I "%1"=="--extract"     ( set DO_IMPORT=0 & set DO_EXTRACT=1 )
if /I "%1"=="--import"      ( set DO_EXTRACT=0 & set DO_IMPORT=1 )
if /I "%1"=="--import"      if not "%2"=="" ( set IMPORT_TARGET=%2 )
if /I "%1"=="--analyze"     ( set DO_EXTRACT=0 & set DO_IMPORT=0 & set DO_ANALYZE=1 )
if /I "%1"=="--analyze"     if not "%2"=="" ( set ANALYZE_TARGET=%2 )
if /I "%1"=="--analyze-cam" ( set DO_EXTRACT=0 & set DO_IMPORT=0 & set DO_ANALYZE_CAM=1 )
if /I "%1"=="--analyze-mc"  ( set DO_EXTRACT=0 & set DO_IMPORT=0 & set DO_ANALYZE_MC=1 )
if /I "%1"=="--secondary"          ( set DO_EXTRACT=0 & set DO_IMPORT=0 & set DO_SECONDARY=1 )
if /I "%1"=="--secondary"          if not "%2"=="" ( set SECONDARY_TARGET=%2 )
if /I "%1"=="--analyze-secondary"  ( set DO_EXTRACT=0 & set DO_IMPORT=0 & set DO_ANALYZE_SECONDARY=1 )

echo ============================================================
echo  FA overlay DLL pipeline
if %DO_EXTRACT%==1    echo   Step 1: extract overlays from FA_2.LIB
if %DO_IMPORT%==1     echo   Step 2: import overlays into Ghidra (%IMPORT_TARGET%)
if %DO_ANALYZE%==1             echo   Step A: analyze overlay DLLs with DumpOverlayDLL (%ANALYZE_TARGET%)
if %DO_ANALYZE_CAM%==1        echo   Step A: analyze CAM DLLs with AnalyzeCAMDLL
if %DO_ANALYZE_MC%==1         echo   Step A: analyze MC DLLs with AnalyzeMCDLL
if %DO_SECONDARY%==1          echo   Step S: import secondary binaries (%SECONDARY_TARGET%)
if %DO_ANALYZE_SECONDARY%==1  echo   Step A: analyze secondary projects with DumpOverlayDLL
echo ============================================================
echo.

if %DO_EXTRACT%==1 (
    call "%~dp0extract_overlays.bat"
    if errorlevel 1 (
        echo ERROR: extraction failed.
        exit /b 1
    )
    echo.
)

if %DO_IMPORT%==1 (
    call "%~dp0import_overlays.bat" %IMPORT_TARGET%
    if errorlevel 1 (
        echo ERROR: import failed.
        exit /b 1
    )
    echo.
)

:: --analyze: run DumpOverlayDLL against overlay projects
if %DO_ANALYZE%==1 (
    set PROJECTS_ROOT=%FA_PROJECT%\overlay_projects
    if /I "%ANALYZE_TARGET%"=="ALL" (
        for %%F in (BI CAM MC HUD LAY FNT MUS) do (
            echo.
            echo --- DumpOverlayDLL [%%F] ---
            call "%~dp0_analyze_overlay.bat" %%F
        )
    ) else (
        echo.
        echo --- DumpOverlayDLL [%ANALYZE_TARGET%] ---
        call "%~dp0_analyze_overlay.bat" %ANALYZE_TARGET%
    )
    echo.
)

:: --analyze-cam: run AnalyzeCAMDLL against the CAM overlay project
if %DO_ANALYZE_CAM%==1 (
    echo.
    echo --- AnalyzeCAMDLL ---
    call "%~dp0_analyze_overlay_script.bat" CAM AnalyzeCAMDLL.java
    echo.
)

:: --analyze-mc: run AnalyzeMCDLL against the MC overlay project
if %DO_ANALYZE_MC%==1 (
    echo.
    echo --- AnalyzeMCDLL ---
    call "%~dp0_analyze_overlay_script.bat" MC AnalyzeMCDLL.java
    echo.
)

:: --analyze-secondary: run DumpOverlayDLL against all secondary projects
if %DO_ANALYZE_SECONDARY%==1 (
    echo.
    echo --- DumpOverlayDLL [IP.EXE] ---
    call "%~dp0_analyze_secondary.bat" IP.EXE "%SEC_ROOT%\ip" fa-ip IP.EXE
    echo.
    echo --- DumpOverlayDLL [WAIL32.DLL] ---
    call "%~dp0_analyze_secondary.bat" WAIL32.DLL "%SEC_ROOT%\wail32" fa-wail32 WAIL32.DLL
    echo.
    echo --- DumpOverlayDLL [msapi.dll] ---
    call "%~dp0_analyze_secondary.bat" msapi.dll "%SEC_ROOT%\msapi" fa-msapi msapi.dll
    echo.
    echo --- DumpOverlayDLL [CDRVDL32.DLL] ---
    call "%~dp0_analyze_secondary.bat" CDRVDL32.DLL "%SEC_ROOT%\cdrom" fa-cdrom CDRVDL32.DLL
    echo.
    echo --- DumpOverlayDLL [CDRVHF32.DLL] ---
    call "%~dp0_analyze_secondary.bat" CDRVHF32.DLL "%SEC_ROOT%\cdrom" fa-cdrom CDRVHF32.DLL
    echo.
    echo --- DumpOverlayDLL [CDRVXF32.DLL] ---
    call "%~dp0_analyze_secondary.bat" CDRVXF32.DLL "%SEC_ROOT%\cdrom" fa-cdrom CDRVXF32.DLL
    echo.
    echo --- DumpOverlayDLL [COMMSC32.DLL] ---
    call "%~dp0_analyze_secondary.bat" COMMSC32.DLL "%SEC_ROOT%\serial" fa-serial COMMSC32.DLL
    echo.
)

:: --secondary: import secondary binaries
if %DO_SECONDARY%==1 (
    echo.
    echo --- Secondary binary import ---
    call "%~dp0import_secondary.bat" %SECONDARY_TARGET%
    if errorlevel 1 (
        echo ERROR: secondary import failed.
        exit /b 1
    )
    echo.
)

echo.
echo ============================================================
echo  Overlay pipeline complete.
echo  Overlay projects  : %FA_PROJECT%\overlay_projects\
echo  Secondary projects: %FA_PROJECT%\secondary_projects\
echo ============================================================
endlocal
