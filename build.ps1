# Build script for C++ Ellipse Benchmark
# Automatically locates and uses MSVC compiler

$ErrorActionPreference = "Stop"

$SOURCE_DIR = Join-Path $PSScriptRoot "src"
$INCLUDE_DIR = Join-Path $PSScriptRoot "include"
$BUILD_DIR = Join-Path $PSScriptRoot "build"
$OUTPUT = Join-Path $BUILD_DIR "ellipse_benchmark.exe"

# Create build directory if it doesn't exist
if (-not (Test-Path $BUILD_DIR)) {
    New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
}

Write-Host "Building Ellipse Benchmark..." -ForegroundColor Cyan

# Locate MSVC installation
$vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if (-not (Test-Path $vcvarsPath)) {
    # Try other common locations
    $altPaths = @(
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    )
    
    foreach ($path in $altPaths) {
        if (Test-Path $path) {
            $vcvarsPath = $path
            break
        }
    }
}

if (-not (Test-Path $vcvarsPath)) {
    Write-Host "Error: Could not find MSVC installation!" -ForegroundColor Red
    Write-Host "Please ensure Visual Studio 2022 with C++ tools is installed." -ForegroundColor Yellow
    exit 1
}

    Write-Host "Using MSVC from: $vcvarsPath" -ForegroundColor Green

# Create results directory if it doesn't exist
if (-not (Test-Path (Join-Path $PSScriptRoot "results"))) {
    New-Item -ItemType Directory -Path (Join-Path $PSScriptRoot "results") | Out-Null
}

# Build using MSVC in a cmd subprocess
$sourceFile = Join-Path $SOURCE_DIR "main.cpp"
$buildCmd = "call `"$vcvarsPath`" && cl /EHsc /std:c++17 /O2 /I`"$INCLUDE_DIR`" `"$sourceFile`" /Fe:`"$OUTPUT`" /Fo:`"$BUILD_DIR\\`""

Write-Host "Compiling..." -ForegroundColor Yellow

# Execute build command
cmd /c $buildCmd

# Check if build succeeded by checking if executable was created
if (Test-Path $OUTPUT) {
    Write-Host "`nBuild successful!" -ForegroundColor Green
    Write-Host "Executable: $OUTPUT" -ForegroundColor Cyan
    Write-Host "`nRunning benchmark..." -ForegroundColor Yellow
    Write-Host ("=" * 60)
    & $OUTPUT
} else {
    Write-Host "`nBuild failed! Executable not created." -ForegroundColor Red
    exit 1
}
