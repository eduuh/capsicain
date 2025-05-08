param(
    [string]$BuildType = "Release",
    [string]$Platform = "x64",
    [switch]$Clean,
    [switch]$Package,
    [switch]$CreateRelease,
    [string]$Version = "v0.97.0",
    [switch]$Sign,
    [string]$CertPath = "code_signing.pfx",
    [switch]$ForceCMakeClean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# SECTION: Parameter Validation
if ($BuildType -ne "Debug" -and $BuildType -ne "Release") { $BuildType = "Release" }
if ($Platform -ne "x64" -and $Platform -ne "Win32") { $Platform = "x64" }

# SECTION: Paths
$scriptPath = $PSScriptRoot
if (-not $scriptPath) { $scriptPath = (Get-Location).Path }
$rootDir = $scriptPath
$buildDir = Join-Path $rootDir "capsicain\build\cmake-$Platform-$BuildType"
$outputDir = Join-Path $buildDir $BuildType

# SECTION: Clean
if ($Clean -and (Test-Path $buildDir)) {
    Remove-Item -Path $buildDir -Recurse -Force
}

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
}

# SECTION: Generator Selection
$generator = "Visual Studio 17 2022"
if (Get-Command vswhere -ErrorAction SilentlyContinue) {
    $vsVersion = & vswhere -latest -property installationVersion
    if ($vsVersion -match '^17') { $generator = "Visual Studio 17 2022" }
    elseif ($vsVersion -match '^16') { $generator = "Visual Studio 16 2019" }
    elseif ($vsVersion -match '^15') { $generator = "Visual Studio 15 2017" }
}
if ($Platform -eq "Win32") {
    $cmakePlatform = "Win32"
} else {
    $cmakePlatform = "x64"
}

# SECTION: CMake Configure
Push-Location $buildDir
$cmakeArgs = @("-G", $generator, "-A", $cmakePlatform, $rootDir)
if ((Test-Path "CMakeCache.txt") -and $ForceCMakeClean) {
    Remove-Item -Path "CMakeCache.txt" -Force
    if (Test-Path "CMakeFiles") { Remove-Item -Path "CMakeFiles" -Recurse -Force }
}
& cmake $cmakeArgs
if ($LASTEXITCODE -ne 0) { Pop-Location; exit 1 }

# SECTION: Build
$cpuCores = $env:NUMBER_OF_PROCESSORS
if (-not $cpuCores) { $cpuCores = 1 }
& cmake --build . --config $BuildType --parallel $cpuCores
if ($LASTEXITCODE -ne 0) { Pop-Location; exit 1 }
Pop-Location

# SECTION: Find Executable
$exePath = ""
$possibleExePaths = @(
    "$outputDir\capsicain.exe",
    "$buildDir\capsicain.exe"
)
foreach ($path in $possibleExePaths) {
    if (Test-Path $path) { $exePath = $path; break }
}
if (-not $exePath) {
    $exeFiles = Get-ChildItem -Path $outputDir -Recurse -Filter "capsicain.exe" -ErrorAction SilentlyContinue
    if ($exeFiles) { $exePath = $exeFiles[0].FullName }
    else { if (-not ($Package -or $CreateRelease)) { exit 1 } }
}

# SECTION: Package
if ($Package) {
    $packageDir = "$outputDir\capsicain-$($Platform.ToLower())-$BuildType"
    $packageZip = "$packageDir.zip"
    if (Test-Path $packageDir) { Remove-Item -Path $packageDir -Recurse -Force }
    New-Item -ItemType Directory -Path $packageDir -Force | Out-Null
    if ($exePath) { Copy-Item -Path $exePath -Destination "$packageDir\" }
    $dllLocations = @(
        "capsicain\interception.dll",
        "$([System.IO.Path]::GetDirectoryName($exePath))\interception.dll",
        "$rootDir\capsicain\interception.dll"
    )
    foreach ($dllPath in $dllLocations) {
        if (Test-Path $dllPath) { Copy-Item -Path $dllPath -Destination "$packageDir\"; break }
    }
    Copy-Item -Path "capsicain\*.ini" -Destination "$packageDir\" -ErrorAction SilentlyContinue
    Copy-Item -Path "*.md" -Destination "$packageDir\" -ErrorAction SilentlyContinue
    Copy-Item -Path "capsicain\documentation.txt" -Destination "$packageDir\" -ErrorAction SilentlyContinue
    if (Get-Command Compress-Archive -ErrorAction SilentlyContinue) {
        Compress-Archive -Path "$packageDir\*" -DestinationPath $packageZip
    }
}

# SECTION: Release
if ($CreateRelease) {
    $releaseDir = "release"
    $releasePackageName = "capsicain-$Version-$Platform"
    $releasePackageDir = Join-Path $releaseDir $releasePackageName
    if (-not (Test-Path $releaseDir)) { New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null }
    if (Test-Path $releasePackageDir) { Remove-Item -Path $releasePackageDir -Recurse -Force }
    New-Item -ItemType Directory -Path $releasePackageDir -Force | Out-Null
    $filesToCopy = @(
        @{Source = $exePath; Destination = $releasePackageDir; Required = $true},
        @{Source = "capsicain\capsicain.ini"; Destination = $releasePackageDir; Required = $true},
        @{Source = "README.md"; Destination = $releasePackageDir; Required = $true},
        @{Source = "capsicain\documentation.txt"; Destination = $releasePackageDir; Required = $false},
        @{Source = "capsicain\capsicain.example.ini"; Destination = $releasePackageDir; Required = $false},
        @{Source = "capsicain\capsicain.kingcon.ini"; Destination = $releasePackageDir; Required = $false},
        @{Source = "capsicain\interception.dll"; Destination = $releasePackageDir; Required = $true}
    )
    foreach ($file in $filesToCopy) {
        if (Test-Path -Path $file.Source) { Copy-Item -Path $file.Source -Destination $file.Destination }
        elseif ($file.Source -eq "capsicain\interception.dll" -and (Test-Path "$([System.IO.Path]::GetDirectoryName($exePath))\interception.dll")) {
            Copy-Item -Path "$([System.IO.Path]::GetDirectoryName($exePath))\interception.dll" -Destination $file.Destination
        }
        elseif ($file.Required) { exit 1 }
    }
    if (Get-Command Compress-Archive -ErrorAction SilentlyContinue) {
        $zipPath = Join-Path $releaseDir "$releasePackageName.zip"
        if (Test-Path $zipPath) { Remove-Item -Path $zipPath -Force }
        Compress-Archive -Path "$releasePackageDir\*" -DestinationPath $zipPath
    }
}