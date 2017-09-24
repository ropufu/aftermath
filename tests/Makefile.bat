@echo off
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set compilerflags=/Od /Zi /EHsc /I.\..\..\aftermath /I.\..\..\opensource
set linkerflags=/OUT:tests.exe
cl.exe %compilerflags% main.cpp /link %linkerflags%
del main.obj
del tests.ilk
del tests.pdb
del vc140.pdb