# src/build.ps1 — convenience wrapper for CMake preset builds (Windows)
# Usage: powershell -File src/build.ps1 [-Preset debug] [-SourceDir src]

param(
    [ValidateSet("debug", "release")]
    [string]$Preset = "debug",

    [string]$SourceDir = $PSScriptRoot
)

$ErrorActionPreference = "Stop"

# On Windows, map preset names to win-* variants
$ActualPreset = "win-$Preset"

$ErrorActionPreference = "Stop"

# Resolve absolute source path
$SourceDir = (Resolve-Path $SourceDir).Path

if (-not (Test-Path "$SourceDir/CMakeLists.txt")) {
    Write-Error "[build] ERROR: CMakeLists.txt not found in $SourceDir"
    exit 1
}

if (-not (Get-Command conan -ErrorAction SilentlyContinue)) {
    Write-Error "[build] ERROR: Conan is required but not found in PATH."
    exit 1
}

$ConanBuildType = if ($Preset -eq "release") { "Release" } else { "Debug" }
$ConanOutputDir = Join-Path $SourceDir "build/$Preset"
$ConanGeneratorsDir = Join-Path $ConanOutputDir "build/$ConanBuildType/generators"

Write-Host "[build] Installing test dependencies with Conan (preset=$ActualPreset, build_type=$ConanBuildType)..."
conan install "$SourceDir/tests" -of "$ConanOutputDir" -s "build_type=$ConanBuildType" --build=missing
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[build] Configuring (preset=$ActualPreset)..."
cmake --preset $ActualPreset -S $SourceDir -UGTest_DIR -DCMAKE_PREFIX_PATH="$ConanGeneratorsDir" -DCMAKE_MODULE_PATH="$ConanGeneratorsDir" -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[build] Building..."
Push-Location $SourceDir
try {
    cmake --build --preset $ActualPreset
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} finally {
    Pop-Location
}

Write-Host "[build] Done."
