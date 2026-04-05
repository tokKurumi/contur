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
$CMakeSourceDir = $SourceDir -replace "\\", "/"

Write-Host "[build] Installing test dependencies with Conan (preset=$ActualPreset, build_type=$ConanBuildType)..."
conan install "$SourceDir/tests" -of "$ConanOutputDir" -s "build_type=$ConanBuildType" -s "compiler.cppstd=20" --build=missing
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Conan's cmake_layout can place generators in different subpaths depending on settings.
$ConanGeneratorsDirCandidates = @(
    (Join-Path $ConanOutputDir "build/generators"),
    (Join-Path $ConanOutputDir "build/$ConanBuildType/generators")
)
$ConanGeneratorsDir = $ConanGeneratorsDirCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $ConanGeneratorsDir) {
    Write-Error "[build] ERROR: Conan generators directory not found under $ConanOutputDir"
    exit 1
}
$CMakeConanGeneratorsDir = $ConanGeneratorsDir -replace "\\", "/"

Write-Host "[build] Configuring (preset=$ActualPreset)..."
cmake --preset $ActualPreset -S $CMakeSourceDir -UGTest_DIR -Uftxui_DIR -DCMAKE_PREFIX_PATH="$CMakeConanGeneratorsDir" -DCMAKE_MODULE_PATH="$CMakeConanGeneratorsDir" -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON
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
