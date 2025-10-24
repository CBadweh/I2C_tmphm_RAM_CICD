rem
rem usage: build.bat
rem Cleans, builds, and flashes Debug configuration
rem

setlocal

set "ws_root=C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\Badweh_Development"
set "build_type=Debug"
set "build_dir=%ws_root%\%build_type%"
set "project_name=Badweh_Development"
set "sn=0670FF3632524B3043205426"
set "flash_tool=C:\Program Files\STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI"
set "flash_start_addr=0x08000000"

cd "%build_dir%"

set PATH=C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.1.0.202410251130\tools\bin;C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845\tools\bin;%PATH%

set compiler_prefix=arm-none-eabi-

echo Cleaning Debug build...
make -j4 clean

echo Building Debug...
make -j4 all

if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)

echo Flashing %project_name%.bin...
"%flash_tool%" --connect port=SWD sn=%sn% --download "%build_dir%\%project_name%.bin" %flash_start_addr% -hardRst