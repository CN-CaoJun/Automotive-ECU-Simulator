[CmdletBinding()]
param(
    [switch]$BootOnly,
    [switch]$AppOnly
)

$ErrorActionPreference = 'Stop'
if ($BootOnly -and $AppOnly) {
    throw 'BootOnly and AppOnly cannot be used together.'
}

$projectRoot = $PSScriptRoot
$toolsRoot = if ($env:EMBEDDED_TOOLS_ROOT) { $env:EMBEDDED_TOOLS_ROOT } else { 'E:\tools' }
$openocd = Get-ChildItem -LiteralPath $toolsRoot -Directory -Filter 'OpenOCD-*' |
    Sort-Object Name -Descending |
    ForEach-Object { Join-Path $_.FullName 'bin\openocd.exe' } |
    Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
    Select-Object -First 1

if (-not $openocd) {
    throw "OpenOCD not found under $toolsRoot\OpenOCD-*\bin\openocd.exe"
}

$bootImage = Join-Path $projectRoot 'boot\build\RTThread-Boot.bin'
$appImage = Join-Path $projectRoot 'build-app\Simulated-GW-ECU.bin'
if (-not $AppOnly -and -not (Test-Path -LiteralPath $bootImage -PathType Leaf)) {
    throw "Boot image not found: $bootImage. Run build-windows.ps1 first."
}
if (-not $BootOnly -and -not (Test-Path -LiteralPath $appImage -PathType Leaf)) {
    throw "App image not found: $appImage. Run build-windows.ps1 first."
}

$scriptsDir = Join-Path $projectRoot 'openocd-scripts'
Push-Location $scriptsDir
try {
    if (-not $AppOnly) {
        & $openocd -f interface/cmsis-dap.cfg -f target/stm32h7x_extern.cfg -f flash_boot.cfg
        if ($LASTEXITCODE -ne 0) { throw 'Boot flash failed.' }
    }

    if (-not $BootOnly) {
        & $openocd -f interface/cmsis-dap.cfg -f target/stm32h7x_extern.cfg -f flash_app.cfg
        if ($LASTEXITCODE -ne 0) { throw 'App flash failed.' }
    }
}
finally {
    Pop-Location
}

Write-Host 'Flash and verify completed successfully.'
