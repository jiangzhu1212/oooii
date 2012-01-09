@echo off
p4 revert "%1"
set EXT=%~x1
if "%EXT%"==".vcxproj" p4 revert "%1.filters"
