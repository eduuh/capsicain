#!/usr/bin/env pwsh
# package.ps1 - Build and package Capsicain using any available toolchain

[CmdletBinding()]
param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Release",
    
    [Parameter(Mandatory=$false)]
    [ValidateSet("x64", "Win32")]
    [string]$Platform = "x64",
    
    [Parameter(Mandatory=$false)]
    [switch]$Clean,
    
    [Parameter(Mandatory=$false)]
    [switch]$ForceCMakeClean
)

# Set up colors for output
$InformationColor = 'Cyan'
$SuccessColor = 'Green'
$WarningColor = 'Yellow'
$ErrorColor = 'Red'

function Write-Step {
    param([string]$Message)
    Write-Host "`n=== $Message ===" -ForegroundColor $InformationColor
}

# Configure paths
$buildDir = "capsicain/build/cmake-$Platform-$BuildType"
$outputDir = "capsicain/build"
$packageDir = "capsicain/build/capsicain-$($Platform.ToLower())-$BuildType"
$packageZip = "$packageDir.zip"

# Make sure output directory exists
if (-not (Test-Path $outputDir)) {
    Write-Step "Creating output directory"
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

# Clean build directory if requested
if ($Clean -and (Test-Path $buildDir)) {
    Write-Step "Cleaning build directory"
    Remove-Item -Path $buildDir -Recurse -Force
    Write-Host "Cleaned $buildDir" -ForegroundColor $SuccessColor
}

# Create build directory if it doesn't exist
if (-not (Test-Path $buildDir)) {
    Write-Step "Creating build directory"
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    Write-Host "Created $buildDir" -ForegroundColor $SuccessColor
} elseif ($ForceCMakeClean) {
    # Clean CMake cache and configuration files
    Write-Step "Cleaning CMake cache"
    if (Test-Path "$buildDir/CMakeCache.txt") {
        Remove-Item -Path "$buildDir/CMakeCache.txt" -Force
    }
    if (Test-Path "$buildDir/CMakeFiles") {
        Remove-Item -Path "$buildDir/CMakeFiles" -Recurse -Force
    }
    Write-Host "Cleaned CMake cache files" -ForegroundColor $SuccessColor
}

# Build configuration
Write-Step "Configuring build with CMake"
Push-Location $buildDir

# Use whatever generator CMake finds
if ($Platform -eq "x64") {
    # Try to determine if Visual Studio is installed and which version
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsInstance = & $vsWhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
        if ($vsInstance) {
            Write-Host "Found Visual Studio at: $vsInstance" -ForegroundColor $InformationColor
            # Use VS version-specific generator
            if (Test-Path "$vsInstance\VC\Auxiliary\Build\vcvars64.bat") {
                Write-Host "Using Visual Studio generator with -A x64" -ForegroundColor $InformationColor
                cmake -DCMAKE_BUILD_TYPE=$BuildType -A x64 ../../..
            } else {
                # Fallback to default generator
                Write-Host "Using default CMake generator" -ForegroundColor $InformationColor
                cmake -DCMAKE_BUILD_TYPE=$BuildType ../../..
            }
        } else {
            # No Visual Studio, use default generator
            Write-Host "No Visual Studio found, using default CMake generator" -ForegroundColor $InformationColor
            cmake -DCMAKE_BUILD_TYPE=$BuildType ../../..
        }
    } else {
        # No vswhere, use default generator
        Write-Host "Using default CMake generator" -ForegroundColor $InformationColor
        cmake -DCMAKE_BUILD_TYPE=$BuildType ../../..
    }
} else {
    # Win32 architecture
    cmake -DCMAKE_BUILD_TYPE=$BuildType -A Win32 ../../..
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: CMake configuration failed." -ForegroundColor $ErrorColor
    Pop-Location
    exit 1
}

# Build the project
Write-Step "Building project ($BuildType)"
cmake --build . --config $BuildType

if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Build failed." -ForegroundColor $ErrorColor
    Pop-Location
    exit 1
}

Pop-Location

# Create package directory
Write-Step "Creating package"
if (Test-Path $packageDir) {
    Remove-Item -Path $packageDir -Recurse -Force
}
New-Item -ItemType Directory -Path $packageDir | Out-Null

# Determine the location of the executable
$exePath = ""
if (Test-Path "$outputDir/$BuildType/capsicain.exe") {
    # Visual Studio / multi-config generator path
    $exePath = "$outputDir/$BuildType/capsicain.exe"
} elseif (Test-Path "$outputDir/capsicain.exe") {
    # Single-config generator path (Ninja, Makefiles)
    $exePath = "$outputDir/capsicain.exe"
} else {
    # Try to find the executable
    $potentialExes = Get-ChildItem -Path $outputDir -Recurse -Filter "capsicain.exe"
    if ($potentialExes.Count -gt 0) {
        $exePath = $potentialExes[0].FullName
        Write-Host "Found executable at: $exePath" -ForegroundColor $SuccessColor
    } else {
        Write-Host "Error: Could not find the built executable." -ForegroundColor $ErrorColor
        exit 1
    }
}

# Copy files to package directory
Write-Host "Copying executable..." -ForegroundColor $InformationColor
Copy-Item -Path $exePath -Destination "$packageDir/"

Write-Host "Copying DLLs and configuration files..." -ForegroundColor $InformationColor
Copy-Item -Path "capsicain/interception.dll" -Destination "$packageDir/" -ErrorAction SilentlyContinue
Copy-Item -Path "capsicain/*.ini" -Destination "$packageDir/" -ErrorAction SilentlyContinue

Write-Host "Copying documentation..." -ForegroundColor $InformationColor
Copy-Item -Path "*.md" -Destination "$packageDir/" -ErrorAction SilentlyContinue

# Create zip archive
Write-Step "Creating zip archive"
$packageZipFullPath = "$outputDir/$(Split-Path $packageZip -Leaf)"
if (Test-Path $packageZipFullPath) {
    Remove-Item -Path $packageZipFullPath -Force
}

if (Get-Command "Compress-Archive" -ErrorAction SilentlyContinue) {
    # Use PowerShell's built-in compression
    Compress-Archive -Path "$packageDir/*" -DestinationPath $packageZipFullPath
} else {
    # Fall back to zip command if available
    if (Get-Command "zip" -ErrorAction SilentlyContinue) {
        zip -r $packageZipFullPath $packageDir
    } else {
        Write-Host "Warning: Could not create zip archive. Compression tools not available." -ForegroundColor $WarningColor
        Write-Host "The package directory '$packageDir' contains all necessary files." -ForegroundColor $InformationColor
    }
}

if (Test-Path $packageZipFullPath) {
    Write-Host "Package created successfully: $packageZipFullPath" -ForegroundColor $SuccessColor
    Write-Host "Package size: $([math]::Round((Get-Item $packageZipFullPath).Length / 1MB, 2)) MB" -ForegroundColor $SuccessColor
} else {
    Write-Host "Package directory created: $packageDir" -ForegroundColor $SuccessColor
}

Write-Host "`nPackaging complete!" -ForegroundColor $SuccessColor