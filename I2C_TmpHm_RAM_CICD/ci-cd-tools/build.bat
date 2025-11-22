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

cd "%build_dir%"

set PATH=C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.1.0.202410251130\tools\bin;C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845\tools\bin;C:/ST/STM32CubeIDE_1.17.0/STM32CubeIDE//plugins/com.st.stm32cube.ide.jre.win64_3.4.0.202409160955/jre/bin/server;C:/ST/STM32CubeIDE_1.17.0/STM32CubeIDE//plugins/com.st.stm32cube.ide.jre.win64_3.4.0.202409160955/jre/bin;C:\windows\system32;C:\windows;C:\windows\System32\Wbem;C:\windows\System32\WindowsPowerShell\v1.0\;C:\windows\System32\OpenSSH\;C:\Program Files\MATLAB\R2024a\bin;C:\Program Files\HP\HP One Agent;C:\Program Files\nodejs\;C:\Program Files\Git\cmd;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\WINDOWS\System32\OpenSSH\;C:\Program Files (x86)\GnuWin32\bin;C:\Program Files\PuTTY\;C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin;C:\Users\Sheen\AppData\Local\Microsoft\WindowsApps;C:\Users\Sheen\AppData\Local\Programs\Microsoft VS Code\bin;C:\Users\Sheen\AppData\Roaming\npm;C:\Users\Sheen\AppData\Local\GitHubDesktop\bin;C:\Program Files (x86)\GnuWin32\bin;C:\Users\Sheen\Desktop\Control System\STM32\STM32_Fastbit\Archive\OpenOCD\0.10.0-13\bin;;C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE

set compiler_prefix=arm-none-eabi-

set "version_file=..\App\gpio-app\version.h"

if [%BUILD_TAG%]==[] goto :do_make

echo #ifndef _VERSION_H_ >"%version_file%"
echo #define _VERSION_H_ >>"%version_file%"
echo #define VERSION "%BUILD_TAG%-%build_type%" >>"%version_file%"
echo #endif >>"%version_file%"

:do_make

make -j4 "%target%"
