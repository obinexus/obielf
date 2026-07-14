param(
    [ValidateSet("Embedded", "Direct")]
    [string]$Mode = "Embedded",
    [string]$BuildDirectory = ""
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($BuildDirectory)) {
    $BuildDirectory =
        Join-Path $ProjectRoot "build-$($Mode.ToLowerInvariant())"
}

$env:Path = "C:\msys64\mingw64\bin;$env:Path"
$Executable = Join-Path $BuildDirectory "fruit-ninja-obielf.exe"

if (-not (Test-Path $Executable)) {
    throw "Game executable not found. Run scripts\build.ps1 first."
}

Push-Location $BuildDirectory
try {
    & $Executable
} finally {
    Pop-Location
}
