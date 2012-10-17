@echo off
p4 revert "%1"
if "%ERRORLEVEL%" NEQ "0" goto err
set EXT=%~x1
if "%EXT%"==".vcxproj" p4 revert "%1.filters"
goto end

:err
echo.
echo ERROR P4 failed to execute. Possible resolutions:
echo.
echo 1. Check that env vars or P4 registry values "P4USER" and "P4PORT" are defined. If not, open P4V.exe, go to Connection then Environment Settings... and click OK. Then restart Visual Studio so it captures the newly set env vars.
echo.
echo 2. Check that $(ItemPath) is specified as the argument list in Visual Studio's External Tool definition.
goto end

:end
