@echo off
setlocal

set "mingw64=mingw64"

if not exist %mingw64% (
	echo Build Error: %mingw64% folder not found
	exit /b 1
)

%mingw64%\bin\g++ src\main.cpp -s -O3 -o out\nest.exe