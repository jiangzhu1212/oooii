@echo off

if "%VS90COMNTOOLS%"=="" goto error

rem copy /Y .\autoexp.dat "%VS90COMNTOOLS%..\packages\debugger\autoexp.dat"
if exists "%VS90COMNTOOLS%..\IDE" copy /Y .\usertype.dat "%VS90COMNTOOLS%..\IDE\usertype.dat"
if exists "%VS100COMNTOOLS%..\IDE" copy /Y .\usertype.dat "%VS100COMNTOOLS%..\IDE\usertype.dat"
regedit .\oNoStepInto.reg

goto end

:error
echo Could not find VS 9.0 install, checked VS90COMNTOOLS.

:end
pause