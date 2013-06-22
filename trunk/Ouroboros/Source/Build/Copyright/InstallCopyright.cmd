@echo off
setlocal enabledelayedexpansion
title Installing Copyright...

rem RUN THIS FROM THE DIRECTORY IT IS IN (so you can click on it from File Explorer).

set SDK_DIR=../../../..
set DIRS=%SDK_DIR%/Ouroboros/Include %SDK_DIR%/Ouroboros/Source

choice /c YNC /m "Do you need the files checked out of Perforce: Yes, No, Cancel?"
if ERRORLEVEL 3 goto Cancel
if ERRORLEVEL 2 goto Execute
if ERRORLEVEL 1 set USEP4="1"

:Execute
if defined USEP4 (
	echo MAKE SURE YOU HAVE NOTHING CHECKED OUT OF PERFORCE!
	echo This will put modifications into the default changelist.
	pause
	call:SIMD "p4 edit" "%DIRS%" "/..."
)

cscript.exe InstallCopyright.vbs header copyright.txt %DIRS%

if defined USEP4 call:SIMD "p4 revert -a" "%DIRS%" "/..."

:Cancel
endlocal
goto end

:SIMD
rem %~1 commandprefix (prepended to each subdir)
rem %~2 subdirs
rem %~3 subdir suffix (appended to each subdir)
setlocal
rem remove double quotes from initial list
set sublist=%~2
set sublist=%sublist:""="%
for %%i in (%sublist%) do (
	set dir=%%~i
	set dir=%dir:"=%
	%~1 "%%~i%~3"
)

endlocal
goto end

:end
