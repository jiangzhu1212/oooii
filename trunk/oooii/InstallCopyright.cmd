@echo off

echo MAKE SURE YOU HAVE NOTHING CHECKED OUT OF PERFORCE!
pause

p4 edit "../inc/oooii/..."
p4 edit "./..."
cscript.exe InstallCopyright.vbs header copyright.txt "..\inc\oooii"
cscript.exe InstallCopyright.vbs header copyright.txt "."
p4 revert -a "../inc/oooii/..."
p4 revert -a "./..."
