@echo off
setlocal

set "mingw64=mingw64"

if not exist %mingw64% (
	echo Build Error: %mingw64% folder not found
	exit /b 1
)

%mingw64%\bin\g++ nest.cpp -o nest-0.5.exe -std=c++20 -Wall -O3 -static -s -mconsole -Iinc