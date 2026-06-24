$ErrorActionPreference = "Stop"

$AppName = "Robot3DRoaming"
$RootDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Action = if ($args.Count -gt 0) { $args[0] } else { "run" }
$BuildType = if ($env:BUILD_TYPE) { $env:BUILD_TYPE } else { "Release" }
$BuildDir = Join-Path $RootDir "build-win"

function Write-Usage {
    Write-Host "Usage: powershell -ExecutionPolicy Bypass -File .\dev.ps1 [run|build|clean|setup-vcpkg|help]"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  powershell -ExecutionPolicy Bypass -File .\dev.ps1"
    Write-Host "  powershell -ExecutionPolicy Bypass -File .\dev.ps1 build"
}

function Require-Command($Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Missing command: $Name"
    }
}

function Get-VcpkgRoot {
    $Candidates = @()
    if ($env:VCPKG_ROOT) {
        $Candidates += $env:VCPKG_ROOT
    }
    $Candidates += "C:\vcpkg"
    $Candidates += (Join-Path $HOME "vcpkg")

    foreach ($Path in $Candidates) {
        if ($Path -and (Test-Path (Join-Path $Path "scripts\buildsystems\vcpkg.cmake"))) {
            return $Path
        }
    }

    return $null
}

function Setup-Vcpkg {
    Require-Command git

    $VcpkgDir = "C:\vcpkg"
    if (-not (Test-Path $VcpkgDir)) {
        git clone https://github.com/microsoft/vcpkg.git $VcpkgDir
    }

    Push-Location $VcpkgDir
    try {
        .\bootstrap-vcpkg.bat
    }
    finally {
        Pop-Location
    }

    Write-Host "vcpkg is ready: $VcpkgDir"
}

function Get-VisualStudioGenerator {
    if ($env:CMAKE_GENERATOR) {
        return $env:CMAKE_GENERATOR
    }

    $VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $VsWhere) {
        $Version = & $VsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion
        $Major = ($Version -split "\.")[0]

        if ($Major -eq "18") {
            return "Visual Studio 18 2026"
        }
        if ($Major -eq "17") {
            return "Visual Studio 17 2022"
        }
        if ($Major -eq "16") {
            return "Visual Studio 16 2019"
        }
    }

    return "Visual Studio 17 2022"
}

function Configure-Project {
    Require-Command cmake

    $VcpkgRoot = Get-VcpkgRoot
    if (-not $VcpkgRoot) {
        throw "Windows build needs vcpkg. Run: powershell -ExecutionPolicy Bypass -File .\dev.ps1 setup-vcpkg"
    }

    $Generator = Get-VisualStudioGenerator
    $Toolchain = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"

    Write-Host "Using CMake generator: $Generator"
    Write-Host "Using vcpkg toolchain: $Toolchain"

    $CmakeArgs = @(
        "-S", $RootDir,
        "-B", $BuildDir,
        "-G", $Generator,
        "-A", "x64",
        "-DCMAKE_TOOLCHAIN_FILE=$Toolchain",
        "-DVCPKG_TARGET_TRIPLET=x64-windows-static"
    )

    & cmake @CmakeArgs
}

function Build-Project {
    Configure-Project
    cmake --build $BuildDir --config $BuildType
}

function Run-Project {
    Build-Project

    $Exe = Join-Path $BuildDir "$BuildType\$AppName.exe"
    if (-not (Test-Path $Exe)) {
        throw "Executable not found: $Exe"
    }

    & $Exe
}

switch ($Action) {
    "run" { Run-Project }
    "build" { Build-Project }
    "clean" {
        Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue
        Write-Host "Removed: $BuildDir"
    }
    "setup-vcpkg" { Setup-Vcpkg }
    "help" { Write-Usage }
    "-h" { Write-Usage }
    "--help" { Write-Usage }
    default {
        Write-Host "Unknown action: $Action"
        Write-Usage
        exit 1
    }
}
