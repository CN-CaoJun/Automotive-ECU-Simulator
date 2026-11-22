[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('Boot', 'App')]
    [string]$Target,
    [switch]$Fresh
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$toolsRoot = if ($env:EMBEDDED_TOOLS_ROOT) { $env:EMBEDDED_TOOLS_ROOT } else { 'E:\tools' }
$gccRoot = if ($env:ARM_GCC_ROOT) { $env:ARM_GCC_ROOT } else { Join-Path $toolsRoot 'xpack-arm-none-eabi-gcc-10.3.1-2.1' }
$cmake = Join-Path $toolsRoot 'cmake-4.4.4-windows-x86_64\bin\cmake.exe'
$make = Join-Path $toolsRoot 'mingw64\bin\make.exe'
$gcc = Join-Path $gccRoot 'bin\arm-none-eabi-gcc.exe'

foreach ($tool in @($cmake, $make, $gcc)) {
    if (-not (Test-Path -LiteralPath $tool -PathType Leaf)) {
        throw "Required tool not found: $tool"
    }
}

$env:ARM_GCC_ROOT = $gccRoot
$env:PATH = @((Split-Path $cmake), (Split-Path $gcc), (Split-Path $make), $env:PATH) -join ';'
$makeForCMake = $make -replace '\\', '/'
$generatorArgs = @('-G', 'Unix Makefiles', "-DCMAKE_MAKE_PROGRAM=$makeForCMake", '-DCMAKE_BUILD_TYPE=Debug')
$sourceDir = Join-Path $projectRoot $Target
$buildDir = Join-Path $sourceDir 'build'
$extraArgs = @()
[object[]]$freshArgs = @()
if ($Fresh) { $freshArgs += '--fresh' }
if ($Target -eq 'App') { $extraArgs += '-DAPP_IN_QSPI=ON' }

& $cmake @freshArgs -S $sourceDir -B $buildDir @generatorArgs @extraArgs
if ($LASTEXITCODE -ne 0) { throw "$Target CMake configuration failed." }

& $cmake --build $buildDir --parallel ([Environment]::ProcessorCount)
if ($LASTEXITCODE -ne 0) { throw "$Target build failed." }

$image = if ($Target -eq 'Boot') { 'RTThread-Boot.bin' } else { 'Simulated-GW-ECU.bin' }
Get-Item (Join-Path $buildDir $image) | Select-Object FullName, Length, LastWriteTime
