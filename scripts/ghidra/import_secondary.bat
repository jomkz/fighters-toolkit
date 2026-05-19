@echo off
:: Imports secondary FA binaries (IP.EXE, WAIL32.DLL, msapi.dll, and serial/CD-ROM DLLs)
:: into separate Ghidra projects under %FA_PROJECT%\secondary_projects\.
::
:: Each binary gets its own project so analyses stay isolated.
::
:: Prerequisite: the binaries must exist at the paths listed in FA_INSTALL
::   default: C:\JANES\Fighters Anthology\
::
:: Usage:
::   import_secondary.bat               -- import all secondary binaries
::   import_secondary.bat IP            -- import IP.EXE only
::   import_secondary.bat WAIL32        -- import WAIL32.DLL only
::   import_secondary.bat MSAPI         -- import msapi.dll only
::
:: Output projects: %FA_PROJECT%\secondary_projects\{ip,wail32,msapi,cdrom,serial}

setlocal

set JAVA_HOME=C:\java\jdk-26.0.1
set GHIDRA_HOME=C:\tools\ghidra_12.1_PUBLIC
set FA_PROJECT=%USERPROFILE%\src\fa
set FA_INSTALL=C:\JANES\Fighters Anthology
set SEC_ROOT=%FA_PROJECT%\secondary_projects
set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

set TARGET=%1
if "%TARGET%"=="" set TARGET=ALL

echo ============================================================
echo  FA secondary binary Ghidra import
echo  Projects: %SEC_ROOT%
echo  Target  : %TARGET%
echo ============================================================
echo.

if not exist "%SEC_ROOT%" mkdir "%SEC_ROOT%"

:: ----------------------------------------------------------------
:: IP.EXE -- multiplayer launcher, feeds AnalyzeNetwork
:: ----------------------------------------------------------------
if /I "%TARGET%"=="IP" goto :do_ip
if /I "%TARGET%"=="ALL" goto :do_ip
goto :wail32
:do_ip
set BIN_IP=%FA_INSTALL%\IP.EXE
if not exist "%BIN_IP%" (
    echo [IP] WARNING: %BIN_IP% not found -- skipping
    goto :wail32
)
echo [IP] Importing %BIN_IP% ...
if not exist "%SEC_ROOT%\ip" mkdir "%SEC_ROOT%\ip"
"%GHIDRA_HOME%\support\analyzeHeadless.bat" "%SEC_ROOT%\ip" fa-ip ^
    -import "%BIN_IP%" -overwrite ^
    -scriptPath "%SCRIPT_DIR%"
echo [IP] Done.
echo.
if /I "%TARGET%"=="IP" goto :done

:wail32
:: ----------------------------------------------------------------
:: WAIL32.DLL -- Miles Sound System (AIL) -- resolve FA.EXE imports
:: ----------------------------------------------------------------
if /I "%TARGET%"=="WAIL32" goto :do_wail32
if /I "%TARGET%"=="ALL" goto :do_wail32
goto :msapi
:do_wail32
set BIN_W32=%FA_INSTALL%\WAIL32.DLL
if not exist "%BIN_W32%" (
    echo [WAIL32] WARNING: %BIN_W32% not found -- skipping
    goto :msapi
)
echo [WAIL32] Importing %BIN_W32% ...
if not exist "%SEC_ROOT%\wail32" mkdir "%SEC_ROOT%\wail32"
"%GHIDRA_HOME%\support\analyzeHeadless.bat" "%SEC_ROOT%\wail32" fa-wail32 ^
    -import "%BIN_W32%" -overwrite ^
    -scriptPath "%SCRIPT_DIR%"
echo [WAIL32] Done.
echo.
if /I "%TARGET%"=="WAIL32" goto :done

:msapi
:: ----------------------------------------------------------------
:: msapi.dll -- MS API wrapper (purpose TBD)
:: ----------------------------------------------------------------
if /I "%TARGET%"=="MSAPI" goto :do_msapi
if /I "%TARGET%"=="ALL" goto :do_msapi
goto :cdrom
:do_msapi
set BIN_MSAPI=%FA_INSTALL%\msapi.dll
if not exist "%BIN_MSAPI%" (
    echo [MSAPI] WARNING: %BIN_MSAPI% not found -- skipping
    goto :cdrom
)
echo [MSAPI] Importing %BIN_MSAPI% ...
if not exist "%SEC_ROOT%\msapi" mkdir "%SEC_ROOT%\msapi"
"%GHIDRA_HOME%\support\analyzeHeadless.bat" "%SEC_ROOT%\msapi" fa-msapi ^
    -import "%BIN_MSAPI%" -overwrite ^
    -scriptPath "%SCRIPT_DIR%"
echo [MSAPI] Done.
echo.
if /I "%TARGET%"=="MSAPI" goto :done

:cdrom
:: ----------------------------------------------------------------
:: CD-ROM driver DLLs (CDRVDL32.DLL / HF32.DLL / XF32.DLL) -- low priority skim
:: ----------------------------------------------------------------
if /I "%TARGET%"=="ALL" (
    if not exist "%SEC_ROOT%\cdrom" mkdir "%SEC_ROOT%\cdrom"
    for %%D in (CDRVDL32.DLL CDRVHF32.DLL CDRVXF32.DLL) do (
        if exist "%FA_INSTALL%\%%D" (
            echo [%%D] Importing ...
            "%GHIDRA_HOME%\support\analyzeHeadless.bat" "%SEC_ROOT%\cdrom" fa-cdrom ^
                -import "%FA_INSTALL%\%%D" -overwrite ^
                -scriptPath "%SCRIPT_DIR%"
            echo [%%D] Done.
        ) else (
            echo [%%D] WARNING: not found at %FA_INSTALL% -- skipping
        )
    )
    echo.
)

:: ----------------------------------------------------------------
:: COMMSC32.DLL -- serial comms wrapper
:: ----------------------------------------------------------------
if /I "%TARGET%"=="ALL" (
    if exist "%FA_INSTALL%\COMMSC32.DLL" (
        echo [COMMSC32] Importing ...
        if not exist "%SEC_ROOT%\serial" mkdir "%SEC_ROOT%\serial"
        "%GHIDRA_HOME%\support\analyzeHeadless.bat" "%SEC_ROOT%\serial" fa-serial ^
            -import "%FA_INSTALL%\COMMSC32.DLL" -overwrite ^
            -scriptPath "%SCRIPT_DIR%"
        echo [COMMSC32] Done.
    ) else (
        echo [COMMSC32] WARNING: not found at %FA_INSTALL% -- skipping
    )
    echo.
)

:done
echo ============================================================
echo  Secondary import complete.  Projects: %SEC_ROOT%
echo ============================================================
endlocal
