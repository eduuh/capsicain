# Build script for Capsicain using CMake
param(
    [string]$BuildType = "Release",
    [string]$Platform = "x64",
    [switch]$Clean,
    [string]$Compiler = "",
    [switch]$Package,
    [switch]$CreateRelease,
    [string]$Version = "v0.97.0",
    [switch]$Sign,
    [string]$CertPath = "code_signing.pfx",
    [switch]$ForceCMakeClean,
    [switch]$StaticLinkInterception
)

# Enable strict mode to catch common errors
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Validate parameters
if ($BuildType -ne "Debug" -and $BuildType -ne "Release") {
    $BuildType = "Release"
    Write-Warning "Invalid BuildType. Set to default: Release"
}

if ($Platform -ne "x64" -and $Platform -ne "Win32") {
    $Platform = "x64"
    Write-Warning "Invalid Platform. Set to default: x64"
}

if ($Compiler -ne "" -and $Compiler -ne "msvc" -and $Compiler -ne "clang" -and $Compiler -ne "mingw") {
    $Compiler = ""
    Write-Warning "Invalid Compiler. Will auto-detect."
}

# Set up colors for output
$InformationColor = 'Cyan'
$SuccessColor = 'Green'
$WarningColor = 'Yellow'
$ErrorColor = 'Red'

# Print script banner
Write-Host "=============================================" -ForegroundColor $InformationColor
Write-Host "   Capsicain Build Script                   " -ForegroundColor $InformationColor
Write-Host "=============================================" -ForegroundColor $InformationColor
Write-Host "Build Type: $BuildType"
Write-Host "Platform: $Platform"
Write-Host "Static Link Interception: $StaticLinkInterception"
Write-Host "=============================================" -ForegroundColor $InformationColor

function Write-Step {
    param([string]$Message)
    Write-Host "`n=== $Message ===" -ForegroundColor $InformationColor
}

function Test-CommandExists {
    param($command)
    try {
        if (Get-Command $command -ErrorAction Stop) {
            return $true
        }
    } catch {
        return $false
    }
    return $false
}

# Check for required tools
Write-Step "Checking requirements"

# Check for CMake
if (Test-CommandExists "cmake") {
    $cmakeVersion = (cmake --version 2>&1 | Select-String -Pattern "version\s+(\d+\.\d+\.\d+)").Matches.Groups[1].Value
    if ($cmakeVersion) {
        Write-Host "Found CMake version $cmakeVersion" -ForegroundColor $SuccessColor
    } else {
        Write-Host "Found CMake but couldn't determine version" -ForegroundColor $WarningColor
    }
} else {
    Write-Host "Error: CMake not found. Please install CMake and add it to your PATH." -ForegroundColor $ErrorColor
    Write-Host "Download from: https://cmake.org/download/" -ForegroundColor $InformationColor
    exit 1
}

# Set up build variables
$scriptPath = $PSScriptRoot
if (-not $scriptPath) {
    $scriptPath = (Get-Location).Path
}
$rootDir = $scriptPath
$buildDir = Join-Path -Path $rootDir -ChildPath "capsicain\build\cmake-$Platform-$BuildType"
$outputDir = Join-Path -Path $rootDir -ChildPath "capsicain\build"

Write-Host "Script directory: $scriptPath" -ForegroundColor $InformationColor
Write-Host "Build directory: $buildDir" -ForegroundColor $InformationColor

if ($Platform -eq "Win32") {
    $cmakePlatform = "Win32"
} else {
    $cmakePlatform = "x64"
}

# Auto-detect compiler if not specified
if ($Compiler -eq "") {
    Write-Host "No compiler specified. Attempting to auto-detect..." -ForegroundColor $InformationColor
    
    if (Test-CommandExists "cl") {
        $Compiler = "msvc"
        Write-Host "Detected Visual C++ compiler" -ForegroundColor $SuccessColor
    }
    elseif (Test-CommandExists "clang++") {
        $Compiler = "clang"
        Write-Host "Detected Clang compiler" -ForegroundColor $SuccessColor
    }
    elseif (Test-CommandExists "g++") {
        $Compiler = "mingw"
        Write-Host "Detected MinGW compiler" -ForegroundColor $SuccessColor
    }
    else {
        Write-Host "No C++ compiler detected in PATH. Checking Visual Studio..." -ForegroundColor $WarningColor
        
        # Try to detect Visual Studio installation
        $vswhereExists = Test-CommandExists "vswhere"
        if ($vswhereExists) {
            $vsInstallPath = & vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
            if ($vsInstallPath) {
                Write-Host "Found Visual Studio installation at: $vsInstallPath" -ForegroundColor $SuccessColor
                $Compiler = "msvc"
            }
        }
        
        # If still no compiler found, default to msvc
        if ($Compiler -eq "") {
            Write-Host "No C++ compiler detected. Will try to use Visual Studio if installed." -ForegroundColor $WarningColor
            $Compiler = "msvc"
        }
    }
}

# Make sure output directory exists
if (-not (Test-Path $outputDir)) {
    Write-Step "Creating output directory"
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

# Clean if requested
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
}

# Check for CMake cache and clean if requested or if changing generators
$cmakeCacheExists = Test-Path "$buildDir/CMakeCache.txt"
$currentGenerator = ""

if ($cmakeCacheExists) {
    $cacheContent = Get-Content "$buildDir/CMakeCache.txt" -Raw
    if ($cacheContent -match 'CMAKE_GENERATOR:INTERNAL=(.+)') {
        $currentGenerator = $matches[1]
        Write-Host "Detected existing CMake generator: $currentGenerator" -ForegroundColor $InformationColor
    }
}

# Determine the generator to use
if ($Compiler -eq "msvc") {
    # Try to detect Visual Studio version
    if (Test-CommandExists "vswhere") {
        $vsVersion = & vswhere -latest -property installationVersion
        if ($vsVersion -match '^(\d+)\.') {
            $vsMajorVersion = [int]$Matches[1]
            if ($vsMajorVersion -ge 17) {
                $generator = "Visual Studio 17 2022"
            } elseif ($vsMajorVersion -eq 16) {
                $generator = "Visual Studio 16 2019"
            } elseif ($vsMajorVersion -eq 15) {
                $generator = "Visual Studio 15 2017"
            } else {
                $generator = "Visual Studio 17 2022"  # Default to latest
            }
            Write-Host "Using Visual Studio generator: $generator" -ForegroundColor $SuccessColor
        } else {
            $generator = "Visual Studio 17 2022"
        }
    } else {
        $generator = "Visual Studio 17 2022"  # Default if vswhere not available
    }
    $extraArgs = @("-A", $cmakePlatform)
} 
elseif ($Compiler -eq "mingw") {
    $generator = "MinGW Makefiles"
    $extraArgs = @("-DCMAKE_BUILD_TYPE=$BuildType")
}
elseif ($Compiler -eq "clang") {
    # Check for build systems in order of preference
    if (Test-CommandExists "ninja") {
        $generator = "Ninja"
        Write-Host "Using Ninja build system with Clang" -ForegroundColor $SuccessColor
    } elseif (Test-CommandExists "nmake") {
        $generator = "NMake Makefiles"
        Write-Host "Using NMake Makefiles with Clang" -ForegroundColor $SuccessColor
    } else {
        $generator = "MinGW Makefiles"
        Write-Host "Using MinGW Makefiles with Clang" -ForegroundColor $SuccessColor
    }
    $extraArgs = @(
        "-DCMAKE_BUILD_TYPE=$BuildType", 
        "-DCMAKE_CXX_COMPILER=clang++", 
        "-DCMAKE_C_COMPILER=clang"
    )
}

# Add static linking option if requested
if ($StaticLinkInterception) {
    $extraArgs += "-DSTATIC_LINK_INTERCEPTION=ON"
    Write-Host "Static linking of interception.dll enabled" -ForegroundColor $InformationColor
} else {
    $extraArgs += "-DSTATIC_LINK_INTERCEPTION=OFF" 
    Write-Host "Dynamic linking of interception.dll enabled" -ForegroundColor $InformationColor
}

# Clean CMake cache if changing generators or if forced
if ($cmakeCacheExists -and ($currentGenerator -ne $generator -or $ForceCMakeClean)) {
    Write-Step "Cleaning CMake cache due to generator change or forced clean"
    Remove-Item -Path "$buildDir/CMakeCache.txt" -Force
    if (Test-Path "$buildDir/CMakeFiles") {
        Remove-Item -Path "$buildDir/CMakeFiles" -Recurse -Force
    }
    Write-Host "CMake cache cleaned" -ForegroundColor $SuccessColor
}

# Configure CMake
Write-Step "Configuring CMake with $Compiler compiler"

# Make sure the build directory exists before changing into it
if (-not (Test-Path -Path $buildDir)) {
    New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
}

# Ensure interception.dll is properly copied during build
Write-Host "Ensuring interception.dll is available for dynamic linking..." -ForegroundColor $InformationColor
$interceptionSource = Join-Path -Path $rootDir -ChildPath "capsicain\interception.dll"
$interceptionBuildDest = Join-Path -Path $buildDir -ChildPath "interception.dll"

# Copy interception.dll to build directory
if (-not $StaticLinkInterception) {
    if (Test-Path $interceptionSource) {
        Write-Host "Found interception.dll at: $interceptionSource" -ForegroundColor $SuccessColor
        
        # Create build directory if it doesn't exist
        if (-not (Test-Path -Path $buildDir)) {
            New-Item -Path $buildDir -ItemType Directory -Force | Out-Null
        }
        
        # Copy DLL to build directory for dynamic linking
        if (-not (Test-Path $interceptionBuildDest)) {
            Copy-Item -Path $interceptionSource -Destination $buildDir -Force
            Write-Host "Copied interception.dll to build directory" -ForegroundColor $SuccessColor
        }
    } else {
        Write-Host "Warning: interception.dll not found at $interceptionSource" -ForegroundColor $WarningColor
        Write-Host "The build might succeed but the resulting executable may not work correctly." -ForegroundColor $WarningColor
    }
} else {
    Write-Host "Using static linking for interception.dll" -ForegroundColor $InformationColor
}

Push-Location $buildDir

# Store the path to the root directory where CMakeLists.txt is located
$cmakeSourceDir = $rootDir

# Build the CMake command
$cmakeArgs = @(
    "-G", $generator
)
$cmakeArgs += $extraArgs
$cmakeArgs += $cmakeSourceDir  # Path to root directory containing CMakeLists.txt

Write-Host "Running: cmake $($cmakeArgs -join ' ')" -ForegroundColor $InformationColor

try {
    & cmake $cmakeArgs
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error: CMake configuration failed with exit code $LASTEXITCODE" -ForegroundColor $ErrorColor
        Pop-Location
        exit 1
    }
} catch {
    Write-Host "Error during CMake configuration: $_" -ForegroundColor $ErrorColor
    Pop-Location
    exit 1
}

# Build the project
Write-Step "Building project ($BuildType)"

# Determine number of processor cores for parallel builds
$cpuCores = 1
if ($env:NUMBER_OF_PROCESSORS) {
    $cpuCores = [int]$env:NUMBER_OF_PROCESSORS
    Write-Host "Detected $cpuCores CPU cores, will use parallel build" -ForegroundColor $InformationColor
}

# Build based on the generator
try {
    if ($generator -match "Visual Studio") {
        Write-Host "Building with Visual Studio..." -ForegroundColor $InformationColor
        & cmake --build . --config $BuildType --parallel $cpuCores
    }
    elseif ($generator -eq "MinGW Makefiles") {
        Write-Host "Building with MinGW..." -ForegroundColor $InformationColor
        if (Test-CommandExists "mingw32-make") {
            & mingw32-make -j $cpuCores
        } elseif (Test-CommandExists "make") {
            & make -j $cpuCores
        } else {
            Write-Host "Error: make/mingw32-make not found." -ForegroundColor $ErrorColor
            Pop-Location
            exit 1
        }
    }
    elseif ($generator -eq "Ninja") {
        Write-Host "Building with Ninja..." -ForegroundColor $InformationColor
        & ninja -j $cpuCores
    }
    elseif ($generator -eq "NMake Makefiles") {
        Write-Host "Building with NMake..." -ForegroundColor $InformationColor
        & nmake
    }
} catch {
    Write-Host "Error during build: $_" -ForegroundColor $ErrorColor
    Pop-Location
    exit 1
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Build failed with exit code $LASTEXITCODE" -ForegroundColor $ErrorColor
    Pop-Location
    exit 1
}

Pop-Location

# Check for build outputs
Write-Step "Checking build outputs"
$exePath = ""

$possibleExePaths = @(
    "$buildDir\$BuildType\capsicain.exe",  # Visual Studio / multi-configuration generator output
    "$buildDir\capsicain.exe",              # Single-configuration generator output
    "$outputDir\$BuildType\capsicain.exe",
    "$outputDir\capsicain.exe"
)

foreach ($path in $possibleExePaths) {
    if (Test-Path $path) {
        $exePath = $path
        Write-Host "Found executable at: $exePath" -ForegroundColor $SuccessColor
        break
    }
}

if (-not $exePath) {
    # Search for the executable recursively as a last resort
    Write-Host "Searching for executable recursively..." -ForegroundColor $WarningColor
    $exeFiles = Get-ChildItem -Path $outputDir -Recurse -Filter "capsicain.exe" -ErrorAction SilentlyContinue
    if ($exeFiles -and $exeFiles.Count -gt 0) {
        $exePath = $exeFiles[0].FullName
        Write-Host "Found executable at: $exePath" -ForegroundColor $SuccessColor
    } else {
        Write-Host "Error: Could not find built executable!" -ForegroundColor $ErrorColor
        if (-not ($Package -or $CreateRelease)) {
            exit 1
        }
    }
}

# Stop here if not packaging or creating a release
if (-not $Package -and -not $CreateRelease) {
    Write-Host "`nBuild process complete!" -ForegroundColor $SuccessColor
    exit 0
}

# Handle packaging
if ($Package) {
    Write-Step "Creating package"
    
    # Configure package paths
    $packageDir = "$outputDir\capsicain-$($Platform.ToLower())-$BuildType"
    $packageZip = "$packageDir.zip"
    
    # Create package directory
    if (Test-Path $packageDir) {
        Remove-Item -Path $packageDir -Recurse -Force
    }
    New-Item -ItemType Directory -Path $packageDir -Force | Out-Null
    
    # Copy files to package directory
    Write-Host "Copying executable..." -ForegroundColor $InformationColor
    if ($exePath -and (Test-Path $exePath)) {
        Copy-Item -Path $exePath -Destination "$packageDir\"
    }
    else {
        Write-Host "Error: Executable not found. Cannot create package." -ForegroundColor $ErrorColor
        exit 1
    }
    
    Write-Host "Copying DLLs and configuration files..." -ForegroundColor $InformationColor
    
    # Check if we need to copy interception.dll (only if not statically linked)
    if (-not $StaticLinkInterception) {
        # Try different locations for interception.dll
        $dllLocations = @(
            "capsicain\interception.dll",
            "$([System.IO.Path]::GetDirectoryName($exePath))\interception.dll",
            "$rootDir\capsicain\interception.dll"
        )
        
        $dllCopied = $false
        foreach ($dllPath in $dllLocations) {
            if (Test-Path $dllPath) {
                Copy-Item -Path $dllPath -Destination "$packageDir\" 
                Write-Host "Copied interception.dll from $dllPath" -ForegroundColor $SuccessColor
                $dllCopied = $true
                break
            }
        }
        
        if (-not $dllCopied) {
            Write-Host "Warning: interception.dll not found. Application may not function correctly." -ForegroundColor $WarningColor
        }
    } else {
        Write-Host "Using statically linked interception library, no DLL needed" -ForegroundColor $SuccessColor
    }
    
    # Copy configuration files
    Copy-Item -Path "capsicain\*.ini" -Destination "$packageDir\" -ErrorAction SilentlyContinue
    
    Write-Host "Copying documentation..." -ForegroundColor $InformationColor
    Copy-Item -Path "*.md" -Destination "$packageDir\" -ErrorAction SilentlyContinue
    Copy-Item -Path "capsicain\documentation.txt" -Destination "$packageDir\" -ErrorAction SilentlyContinue
    
    # Create zip archive
    Write-Step "Creating zip archive"
    $packageZipFullPath = "$outputDir\$(Split-Path $packageZip -Leaf)"
    if (Test-Path $packageZipFullPath) {
        Remove-Item -Path $packageZipFullPath -Force
    }
    
    if (Test-CommandExists "Compress-Archive") {
        # Use PowerShell's built-in compression
        Compress-Archive -Path "$packageDir\*" -DestinationPath $packageZipFullPath
    } else {
        # Fall back to zip command if available
        if (Test-CommandExists "zip") {
            Push-Location $outputDir
            & zip -r $packageZipFullPath $packageDir
            Pop-Location
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
}

# Handle release creation
if ($CreateRelease) {
    Write-Step "Creating release package: capsicain-$Version-$Platform"
    
    # Setup release directory structure
    $releaseDir = "release"
    $releasePackageName = "capsicain-$Version-$Platform"
    $releasePackageDir = Join-Path -Path $releaseDir -ChildPath $releasePackageName
    
    # Create release directory
    if (-not (Test-Path -Path $releaseDir)) {
        New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null
    }
    
    # Remove existing package directory if it exists
    if (Test-Path -Path $releasePackageDir) {
        Remove-Item -Path $releasePackageDir -Recurse -Force
    }
    
    # Create new package directory
    New-Item -ItemType Directory -Path $releasePackageDir -Force | Out-Null
    
    # Files to copy to the release package
    $filesToCopy = @(
        @{Source = $exePath; Destination = $releasePackageDir; Required = $true}
        @{Source = "capsicain\capsicain.ini"; Destination = $releasePackageDir; Required = $true}
        @{Source = "README.md"; Destination = $releasePackageDir; Required = $true}
        @{Source = "capsicain\documentation.txt"; Destination = $releasePackageDir; Required = $false}
        @{Source = "capsicain\capsicain.example.ini"; Destination = $releasePackageDir; Required = $false}
        @{Source = "capsicain\capsicain.kingcon.ini"; Destination = $releasePackageDir; Required = $false}
    )
    
    # Add interception.dll to the list if not statically linked
    if (-not $StaticLinkInterception) {
        $filesToCopy += @{Source = "capsicain\interception.dll"; Destination = $releasePackageDir; Required = $true}
    }
    
    # Copy each file to the release directory
    foreach ($file in $filesToCopy) {
        if (Test-Path -Path $file.Source) {
            Copy-Item -Path $file.Source -Destination $file.Destination
            Write-Host "Copied $($file.Source) to release package" -ForegroundColor $SuccessColor
        } 
        elseif ($file.Source -eq "capsicain\interception.dll" -and (Test-Path "$([System.IO.Path]::GetDirectoryName($exePath))\interception.dll")) {
            # Try to find interception.dll in the build output directory
            Copy-Item -Path "$([System.IO.Path]::GetDirectoryName($exePath))\interception.dll" -Destination $file.Destination
            Write-Host "Copied interception.dll from build directory to release package" -ForegroundColor $SuccessColor
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
        if (-not (Test-Path $CertPath)) {
            Write-Host "Warning: No signing certificate found at $CertPath, skipping signing" -ForegroundColor $WarningColor
        }
        else {
            # Ask for password if needed
            $password = Read-Host "Enter certificate password" -AsSecureString
            
            try {
                # Sign the executable
                $exeInPackage = Join-Path -Path $releasePackageDir -ChildPath "capsicain.exe"
                Set-AuthenticodeSignature -FilePath $exeInPackage -Certificate (Get-PfxCertificate -FilePath $CertPath -Password $password)
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
    
    # Create the zip file using the most appropriate method available
    try {
        if (Test-CommandExists "Compress-Archive") {
            # Use PowerShell's built-in compression
            Compress-Archive -Path "$releasePackageDir\*" -DestinationPath $zipPath
        } else {
            # Fall back to .NET Framework method
            Add-Type -AssemblyName System.IO.Compression.FileSystem
            [System.IO.Compression.ZipFile]::CreateFromDirectory($releasePackageDir, $zipPath)
        }
        
        if (Test-Path -Path $zipPath) {
            Write-Host "Successfully created ZIP archive: $zipPath" -ForegroundColor $SuccessColor
            
            # Calculate and display checksums
            Write-Host "`nFile hashes for verification:" -ForegroundColor $InformationColor
            $sha256 = Get-FileHash -Path $zipPath -Algorithm SHA256
            $md5 = Get-FileHash -Path $zipPath -Algorithm MD5
            
            Write-Host "SHA256: $($sha256.Hash)" -ForegroundColor $SuccessColor
            Write-Host "MD5: $($md5.Hash)" -ForegroundColor $SuccessColor
            
            Write-Host "`nRelease files are available in: $releasePackageDir" -ForegroundColor $SuccessColor
        } else {
            throw "ZIP file was not created"
        }
    } catch {
        Write-Host "Error: Failed to create ZIP archive: $_" -ForegroundColor $ErrorColor
        Write-Host "Release files are still available in: $releasePackageDir" -ForegroundColor $InformationColor
    }
}

Write-Host "`nBuild pipeline complete!" -ForegroundColor $SuccessColor