rem
rem usage: build build-dir {Debug|Release} {all|clean}
rem

setlocal

if not [%3]==[] goto :get_args
echo "Insufficient arguments"
exit /b 1

:get_args

set "build_dir=%1"
set "build_type=%2"
set "target=%3"

echo [DEBUG build.bat] Received arguments:
echo [DEBUG build.bat]   build_dir=%build_dir%
echo [DEBUG build.bat]   build_type=%build_type%
echo [DEBUG build.bat]   target=%target%

if not exist "%build_dir%" (
    echo [DEBUG build.bat] Creating directory: %build_dir%
    mkdir "%build_dir%"
) else (
    echo [DEBUG build.bat] Directory already exists: %build_dir%
)

rem Check if directory was created successfully
if not exist "%build_dir%" (
    echo ERROR: Failed to create build directory: %build_dir%
    exit /b 1
)

cd /d "%build_dir%"
if errorlevel 1 (
    echo ERROR: Failed to change to directory: %build_dir%
    exit /b 1
)

rem Check if makefile exists
if not exist "makefile" (
    echo ERROR: makefile not found in %build_dir%
    echo Please ensure the Debug directory with makefile is checked into git or generated before building.
    exit /b 1
)

rem Preprocess makefile to replace absolute paths with relative paths
echo [DEBUG build.bat] Preprocessing makefile to fix absolute paths...

rem Backup original makefile
copy /Y makefile makefile.original >nul
if errorlevel 1 (
    echo ERROR: Failed to backup makefile
    exit /b 1
)

rem Use PowerShell to replace absolute paths in makefile
rem This replaces any absolute path containing the project name with a relative path
powershell -Command "(Get-Content makefile.original) -replace 'C:\\Users\\[^\\]+\\Desktop\\Embedded_System\\gene_Baremetal_I2CTmphm_RAM_CICD\\I2C_TmpHm_RAM_CICD\\', '../' -replace 'C:\\ProgramData\\Jenkins\\.jenkins\\workspace\\I2C_TmpHm_RAM_CICD\\', '../' | Set-Content makefile"

if errorlevel 1 (
    echo ERROR: Failed to preprocess makefile
    rem Restore original makefile
    copy /Y makefile.original makefile >nul
    del makefile.original
    exit /b 1
)

echo [DEBUG build.bat] Makefile preprocessing completed successfully

set PATH=C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.1.0.202410251130\tools\bin;C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845\tools\bin;C:/ST/STM32CubeIDE_1.17.0/STM32CubeIDE//plugins/com.st.stm32cube.ide.jre.win64_3.4.0.202409160955/jre/bin/server;C:/ST/STM32CubeIDE_1.17.0/STM32CubeIDE//plugins/com.st.stm32cube.ide.jre.win64_3.4.0.202409160955/jre/bin;C:\windows\system32;C:\windows;C:\windows\System32\Wbem;C:\windows\System32\WindowsPowerShell\v1.0\;C:\windows\System32\OpenSSH\;C:\Program Files\MATLAB\R2024a\bin;C:\Program Files\HP\HP One Agent;C:\Program Files\nodejs\;C:\Program Files\Git\cmd;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\WINDOWS\System32\OpenSSH\;C:\Program Files (x86)\GnuWin32\bin;C:\Program Files\PuTTY\;C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin;C:\Users\Sheen\AppData\Local\Microsoft\WindowsApps;C:\Users\Sheen\AppData\Local\Programs\Microsoft VS Code\bin;C:\Users\Sheen\AppData\Roaming\npm;C:\Users\Sheen\AppData\Local\GitHubDesktop\bin;C:\Program Files (x86)\GnuWin32\bin;C:\Users\Sheen\Desktop\Control System\STM32\STM32_Fastbit\Archive\OpenOCD\0.10.0-13\bin;;C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE

set compiler_prefix=arm-none-eabi-

if [%BUILD_TAG%]==[] goto :do_make

:do_make

make -j4 "%target%"
set make_exit_code=%errorlevel%

rem Restore original makefile after build (whether success or failure)
echo [DEBUG build.bat] Restoring original makefile...
if exist "makefile.original" (
    copy /Y makefile.original makefile >nul
    del makefile.original
    echo [DEBUG build.bat] Original makefile restored successfully
) else (
    echo [WARNING build.bat] makefile.original not found, skipping restoration
)

rem Check if make command failed
if %make_exit_code% neq 0 (
    echo ERROR: Build failed with exit code %make_exit_code%
    exit /b %make_exit_code%
)

echo [DEBUG build.bat] Build completed successfully