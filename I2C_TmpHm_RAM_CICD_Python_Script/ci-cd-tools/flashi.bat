rem @echo off
set "usage=usage: flashi [{Debug|Release}]"

setlocal

set "build_type=Debug"
if not [%1]==[] set "build_type=%1"

set "ws_root=C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\I2C_TmpHm_RAM_CICD"
set "sn=0670FF3632524B3043205426"
set "image_file=%ws_root%\%build_type%\I2C_TmpHm_RAM_CICD.bin"

"%ws_root%\ci-cd-tools\flash.bat" %sn% "%image_file%"
