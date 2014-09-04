@echo off

set esar_sartol=sartol.exe
set esar_bim2bin=bim2bin.exe
set esar_wce=wce.exe

if exist %1.sar goto end

if %2.==. goto all
%esar_wce% %esar_sartol% e %1.sar %1 !1 #b=%1 %2 %3 %4 %5 %6 %7 %8 %9
goto tek5

:all
%esar_wce% %esar_sartol% e %1.sar %1 !1 #b=%1 *

:tek5
%esar_wce% %esar_bim2bin% -osacmp -tek5 in:%1.sar out:%1.sar

:end