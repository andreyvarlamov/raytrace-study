@echo off

pushd ..\bin

cl ..\src\raytrace.cpp

popd

..\bin\raytrace.exe
