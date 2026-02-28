@echo off
REM Simple build script for C++ Ellipse Benchmark
REM Requires a C++ compiler (MSVC, MinGW, or Clang)

setlocal enabledelayedexpansion

set "SOURCE_DIR=%~dp0src"
set "INCLUDE_DIR=%~dp0include"
set "BUILD_DIR=%~dp0build"
set "OUTPUT=%~dp0build\ellipse_benchmark.exe"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo Building Ellipse Benchmark...

REM Try to find MSVC compiler
where cl.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo Using MSVC compiler
    cl /std:c++latest /O2 /I"%INCLUDE_DIR%" "%SOURCE_DIR%\main.cpp" /Fe:"%OUTPUT%"
    if %ERRORLEVEL% equ 0 (
        echo Build successful!
        echo Running benchmark...
        "%OUTPUT%"
    ) else (
        echo Build failed!
    )
    goto :end
)

REM Try MinGW
where g++.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo Using MinGW compiler
    g++ -std=c++17 -O3 -I"%INCLUDE_DIR%" "%SOURCE_DIR%\main.cpp" -o "%OUTPUT%"
    if %ERRORLEVEL% equ 0 (
        echo Build successful!
        echo Running benchmark...
        "%OUTPUT%"
    ) else (
        echo Build failed!
    )
    goto :end
)

REM Try Clang
where clang++.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo Using Clang compiler
    clang++ -std=c++17 -O3 -I"%INCLUDE_DIR%" "%SOURCE_DIR%\main.cpp" -o "%OUTPUT%"
    if %ERRORLEVEL% equ 0 (
        echo Build successful!
        echo Running benchmark...
        "%OUTPUT%"
    ) else (
        echo Build failed!
    )
    goto :end
)

echo Error: No supported C++ compiler found!
echo Please install MSVC, MinGW, or Clang

:end
endlocal
