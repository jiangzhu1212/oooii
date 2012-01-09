@echo off
if exist "%VS90COMNTOOLS%..\IDE" echo Found VS90COMNTOOLS, installing usertype.dat...
rem if exist "%VS90COMNTOOLS%..\IDE" copy /Y .\autoexp.dat "%VS90COMNTOOLS%..\packages\debugger\autoexp.dat"
if exist "%VS90COMNTOOLS%..\IDE" copy /Y .\usertype.dat "%VS90COMNTOOLS%..\IDE\usertype.dat"

if exist "%VS100COMNTOOLS%..\IDE" echo Found VS100COMNTOOLS, installing usertype.dat...
rem if exist "%VS100COMNTOOLS%..\IDE" copy /Y .\autoexp.dat "%VS100COMNTOOLS%..\packages\debugger\autoexp.dat"
if exist "%VS100COMNTOOLS%..\IDE" copy /Y .\usertype.dat "%VS100COMNTOOLS%..\IDE\usertype.dat"
regedit .\oNoStepInto.reg
pause
