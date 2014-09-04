@echo off

set dsar_sartol=sartol.exe
set dsar_bpath="%USERPROFILE%/デスクトップ/"
set dsar_autorun="%SystemRoot%\explorer.exe"

rem －例－
rem set dsar_bpath="%USERPROFILE%/デスクトップ/"
rem set dsar_bpath="%USERPROFILE%/デスクトップ"
rem set dsar_bpath=..@arcpath/
rem set dsar_bpath=..@arcpath

rem －例－
rem set dsar_autorun="%SystemRoot%\explorer.exe"
rem set dsar_autorun=
rem 注意！dsar_bpathではパスの区切りに\を使うこと

:loop
if %1.==. goto end
%dsar_sartol% d %1 %dsar_bpath% %dsar_autorun%
shift
goto loop
:end