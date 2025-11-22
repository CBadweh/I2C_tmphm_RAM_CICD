rem @echo off
set "usage=usage: buildi [{Debug|Release} {all|clean}]"

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
set 
set "build_type=%1"
set "target=%2"

:set_build_dir
set "ws_root=C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\I2C_TmpHm_RAM_CICD"

set "build_dir=%ws_root%\%build_type%"

"%ws_root%\ci-cd-tools\build.bat" "%build_dir%" %build_type% %target%
