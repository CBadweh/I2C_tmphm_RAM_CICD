@echo on
set "usage=usage: buildi [{Debug|Release} {all|clean}]"

echo [DEBUG] buildi.bat started
echo [DEBUG] Arguments received: %1 %2 %3

setlocal

if not [%1]==[] goto :check_for_args
set "build_type=Debug"
set "target=all"
goto :set_build_dir

:check_for_args
if not [%2]==[] goto :set_type_target

echo "%usage%"
exit /b 1

:set_type_target
set "build_type=%1"
set "target=%2"

:set_build_dir
rem Use Jenkins WORKSPACE if available, otherwise use hardcoded local path
echo [DEBUG] Checking WORKSPACE variable...
if defined WORKSPACE (
    echo [DEBUG] WORKSPACE is defined: %WORKSPACE%
    set "ws_root=%WORKSPACE%\I2C_TmpHm_RAM_CICD"
) else (
    echo [DEBUG] WORKSPACE is NOT defined, using hardcoded path
    set "ws_root=C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\I2C_TmpHm_RAM_CICD"
)

set "build_dir=%ws_root%\%build_type%"

rem Use %~dp0 to get the directory where this script is located (more reliable than absolute path)
set "script_dir=%~dp0"

rem Debug output
echo [DEBUG] Workspace root: %ws_root%
echo [DEBUG] Build directory: %build_dir%
echo [DEBUG] Script directory: %script_dir%
echo [DEBUG] Build type: %build_type%
echo [DEBUG] Target: %target%
echo [DEBUG] Checking if build.bat exists...
if exist "%script_dir%build.bat" (
    echo [DEBUG] build.bat found at: %script_dir%build.bat
) else (
    echo [ERROR] build.bat NOT found at: %script_dir%build.bat
    exit /b 1
)

echo [DEBUG] Calling build.bat...
call "%script_dir%build.bat" "%build_dir%" %build_type% %target%
if errorlevel 1 (
    echo [ERROR] build.bat returned error code: %errorlevel%
    exit /b %errorlevel%
)