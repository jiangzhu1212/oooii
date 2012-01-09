@echo off
p4 edit "%1"
set EXT=%~x1
if "%EXT%"==".vcxproj" p4 edit "%1.filters"
