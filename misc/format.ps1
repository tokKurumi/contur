# format.ps1 - Format all C++ source and header files according to .clang-format

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$SrcDir = Join-Path $ProjectRoot "src"

if (-not (Test-Path $SrcDir)) {
    Write-Error "Error: src directory not found at $SrcDir"
    exit 1
}

if (-not (Get-Command clang-format -ErrorAction SilentlyContinue)) {
    Write-Error "Error: clang-format not found. Please install clang-format."
    exit 1
}

Write-Host "Formatting C++ files in $SrcDir..."

Get-ChildItem -Path $SrcDir -Recurse -Include "*.cpp", "*.h" |
    Where-Object { $_.FullName -notmatch '[/\\]build[/\\]' } |
    ForEach-Object {
        Write-Host "  Formatting: $($_.FullName)"
        clang-format -i $_.FullName
    }

Write-Host "`u{2713} Formatting complete"
