[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
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
$generatorArgs = @('-G', 'Unix Makefiles', "-DCMAKE_MAKE_PROGRAM=$makeForCMake")
$jobs = [Environment]::ProcessorCount

Push-Location $projectRoot
try {
    & $cmake -S boot -B boot/build @generatorArgs -DCMAKE_BUILD_TYPE=Release
    if ($LASTEXITCODE -ne 0) { throw 'Boot CMake configuration failed.' }

    & $cmake --build boot/build --parallel $jobs
    if ($LASTEXITCODE -ne 0) { throw 'Boot build failed.' }

    & $cmake -S . -B build-app @generatorArgs -DAPP_IN_QSPI=ON
    if ($LASTEXITCODE -ne 0) { throw 'App CMake configuration failed.' }

    & $cmake --build build-app --parallel $jobs
    if ($LASTEXITCODE -ne 0) { throw 'App build failed.' }
}
finally {
    Pop-Location
}

Get-Item boot/build/RTThread-Boot.bin, build-app/Simulated-GW-ECU.bin |
    Select-Object FullName, Length, LastWriteTime
