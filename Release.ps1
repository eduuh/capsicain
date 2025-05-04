# Release packaging script for Capsicain

[CmdletBinding()]
param(
    [Parameter(Mandatory=$false)]
    [string]$Version = "v0.97.0",
    
    [Parameter(Mandatory=$false)]
    [ValidateSet("x64", "Win32")]
    [string]$Platform = "x64",
    
    [Parameter(Mandatory=$false)]
    [switch]$BuildFirst,
    
    [Parameter(Mandatory=$false)]
    [switch]$Sign
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
$buildDir = "build-$Platform-Release"
if ((Get-Command "cl.exe" -ErrorAction SilentlyContinue)) {
    $exePath = "$buildDir\Release\capsicain.exe"
} else {
    $exePath = "$buildDir\capsicain.exe"
}

# Build if requested or if exe doesn't exist
if ($BuildFirst -or -not (Test-Path $exePath)) {
    Write-Step "Building Capsicain before packaging"
    & .\Build.ps1 -BuildType Release -Platform $Platform
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error: Build failed, cannot create release package." -ForegroundColor $ErrorColor
        exit 1
    }
}

# Verify executable exists
if (-not (Test-Path $exePath)) {
    Write-Host "Error: Cannot find executable at $exePath" -ForegroundColor $ErrorColor
    Write-Host "Please build the project first or use -BuildFirst." -ForegroundColor $ErrorColor
    exit 1
}

# Setup release directory structure
$releaseDir = "release"
$releasePackageName = "capsicain-$Version-$Platform"
$releasePackageDir = Join-Path -Path $releaseDir -ChildPath $releasePackageName

Write-Step "Creating release package: $releasePackageName"

# Create release directory
if (-not (Test-Path -Path $releaseDir)) {
    New-Item -ItemType Directory -Path $releaseDir | Out-Null
}

# Remove existing package directory if it exists
if (Test-Path -Path $releasePackageDir) {
    Remove-Item -Path $releasePackageDir -Recurse -Force
}

# Create new package directory
New-Item -ItemType Directory -Path $releasePackageDir | Out-Null

# Files to copy to the release package
$filesToCopy = @(
    @{Source = $exePath; Destination = $releasePackageDir; Required = $true}
    @{Source = "capsicain/capsicain.ini"; Destination = $releasePackageDir; Required = $true}
    @{Source = "README.md"; Destination = $releasePackageDir; Required = $true}
    @{Source = "capsicain/interception.dll"; Destination = $releasePackageDir; Required = $true}
    @{Source = "capsicain/documentation.txt"; Destination = $releasePackageDir; Required = $false}
    @{Source = "capsicain/capsicain.example.ini"; Destination = $releasePackageDir; Required = $false}
    @{Source = "capsicain/capsicain.kingcon.ini"; Destination = $releasePackageDir; Required = $false}
)

# Copy each file to the release directory
foreach ($file in $filesToCopy) {
    if (Test-Path -Path $file.Source) {
        Copy-Item -Path $file.Source -Destination $file.Destination
        Write-Host "Copied $($file.Source) to release package" -ForegroundColor $SuccessColor
    } 
    elseif ($file.Required) {
        Write-Host "ERROR: Required file $($file.Source) not found!" -ForegroundColor $ErrorColor
        exit 1
    }
    else {
        Write-Host "Warning: Optional file $($file.Source) not found" -ForegroundColor $WarningColor
    }
}

# Sign the executable if requested
if ($Sign) {
    Write-Step "Signing executable"
    
    # Check if there's a certificate
    $certPath = "code_signing.pfx"
    if (-not (Test-Path $certPath)) {
        Write-Host "Warning: No signing certificate found at $certPath, skipping signing" -ForegroundColor $WarningColor
    }
    else {
        # Ask for password if needed
        $password = Read-Host "Enter certificate password" -AsSecureString
        
        try {
            # Sign the executable
            $exeInPackage = Join-Path -Path $releasePackageDir -ChildPath "capsicain.exe"
            Set-AuthenticodeSignature -FilePath $exeInPackage -Certificate (Get-PfxCertificate -FilePath $certPath -Password $password)
            Write-Host "Successfully signed executable" -ForegroundColor $SuccessColor
        }
        catch {
            Write-Host "Error: Failed to sign executable: $_" -ForegroundColor $ErrorColor
            # Continue without signing
        }
    }
}

# Create ZIP archive
$zipPath = Join-Path -Path $releaseDir -ChildPath "$releasePackageName.zip"
Write-Step "Creating ZIP archive: $zipPath"

# Remove existing zip if it exists
if (Test-Path -Path $zipPath) {
    Remove-Item -Path $zipPath -Force
}

# Create the zip file
Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::CreateFromDirectory($releasePackageDir, $zipPath)

if (Test-Path -Path $zipPath) {
    Write-Host "Successfully created ZIP archive: $zipPath" -ForegroundColor $SuccessColor
    
    # Calculate and display checksums
    Write-Host "`nFile hashes for verification:" -ForegroundColor $InformationColor
    $sha256 = Get-FileHash -Path $zipPath -Algorithm SHA256
    $md5 = Get-FileHash -Path $zipPath -Algorithm MD5
    
    Write-Host "SHA256: $($sha256.Hash)" -ForegroundColor $SuccessColor
    Write-Host "MD5: $($md5.Hash)" -ForegroundColor $SuccessColor
    
    Write-Host "`nRelease files are available in: $releasePackageDir" -ForegroundColor $SuccessColor
}
else {
    Write-Host "Error: Failed to create ZIP archive" -ForegroundColor $ErrorColor
}