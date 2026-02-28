@echo off
REM Test script for C# ellipse fast algorithms

echo Compiling C# port...
csc /out:EllipseFast.exe EllipseFast.cs

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilation successful! Running tests...
    echo.
    EllipseFast.exe
) else (
    echo.
    echo Compilation failed. Make sure C# compiler is installed.
    echo.
    echo For Visual Studio users, run this from "Developer Command Prompt"
    echo Or install .NET SDK from: https://dotnet.microsoft.com/download
)

pause
