param(
    [ValidateSet("Embedded", "Direct")]
    [string]$Mode = "Embedded",
    [switch]$SkipTests
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$MsysRoot = "C:\msys64\mingw64"
$Compiler = Join-Path $MsysRoot "bin\gcc.exe"
$Make = Join-Path $MsysRoot "bin\mingw32-make.exe"
$BuildRoot = Join-Path $ProjectRoot "build-$($Mode.ToLowerInvariant())"

if (-not (Test-Path $Compiler)) {
    throw "MSYS2 MinGW64 GCC was not found at $Compiler"
}

$Embed = if ($Mode -eq "Embedded") { "ON" } else { "OFF" }
$env:Path = "$(Join-Path $MsysRoot 'bin');$env:Path"

cmake `
    -S $ProjectRoot `
    -B $BuildRoot `
    -G "MinGW Makefiles" `
    "-DCMAKE_C_COMPILER=$Compiler" `
    "-DCMAKE_MAKE_PROGRAM=$Make" `
    "-DCMAKE_PREFIX_PATH=$MsysRoot" `
    "-DFRUIT_OBIELF_EMBED_ASSETS=$Embed" `
    "-DCMAKE_BUILD_TYPE=Debug"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build $BuildRoot --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if (-not $SkipTests) {
    ctest --test-dir $BuildRoot --output-on-failure
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

Write-Host "Built $Mode mode at $BuildRoot"
