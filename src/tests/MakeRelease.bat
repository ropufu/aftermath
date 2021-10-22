@echo off
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set compilerflags=/std:c++latest /O2 /W4 /WX /EHsc /permissive- /bigobj /I.\..\..\..\..\include /Fe:tests.exe
cl.exe %compilerflags% main.cpp
IF EXIST main.obj del main.obj
IF EXIST main.d del main.d
IF EXIST main.o del main.o
