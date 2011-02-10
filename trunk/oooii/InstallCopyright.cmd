@echo off

echo MAKE SURE YOU HAVE NOTHING CHECKED OUT OF PERFORCE!
pause

rem p4 edit "../inc/oooii/..."
rem p4 edit "./..."
cscript.exe InstallCopyright.vbs header copyright.txt "..\inc\oooii"
cscript.exe InstallCopyright.vbs header copyright.txt "."
rem p4 revert -a "../inc/oooii/..."
rem p4 revert -a "./..."
