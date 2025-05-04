# Building Capsicain

Capsicain can be built with a fully open-source toolchain on Windows, without requiring Visual Studio. The project uses CMake as its build system and supports Clang, MinGW-GCC, or Visual Studio Build Tools.

## Quick Start with Open-Source Tools

To quickly install the required tools and build Capsicain with Clang:

1. **Install the required tools** (requires admin rights):
   ```powershell
   # Run as Administrator
   .\install-build-tools.ps1
   ```

2. **Build the project**:
   ```powershell
   .\BuildWithClang.ps1
   ```

That's it! The executable will be created in the `build-clang-x64-Release` directory.

## Open-Source Build Tools

Our recommended open-source toolchain consists of:

- **Clang/LLVM**: Modern C++ compiler with excellent diagnostics
- **CMake**: Cross-platform build system
- **Ninja**: Fast, lightweight build executor
- **ccache** (optional): For faster rebuilds

### Automatic Installation

The `install-build-tools.ps1` script will install all necessary tools using Chocolatey. Run it with administrator privileges:

```powershell
# Run as Administrator
.\install-build-tools.ps1
```

If you want to skip specific tools:
```powershell
.\install-build-tools.ps1 -SkipCCache -SkipNinja
```

### Manual Installation

If you prefer to install the tools manually:

1. **Install LLVM/Clang**:
   - Download from [LLVM Releases](https://github.com/llvm/llvm-project/releases/)
   - Add the `bin` directory to your PATH

2. **Install CMake**:
   - Download from [cmake.org](https://cmake.org/download/)
   - Ensure to add CMake to your PATH during installation

3. **Install Ninja**:
   - Download from [ninja-build.org](https://github.com/ninja-build/ninja/releases)
   - Place the ninja.exe in a directory that's in your PATH

4. **Install ccache** (optional):
   - Download from [ccache releases](https://github.com/ccache/ccache/releases)
   - Add to your PATH

## Building with Clang (Recommended)

The `BuildWithClang.ps1` script configures and builds the project using Clang and Ninja:

```powershell
# Basic usage (Release build for x64)
.\BuildWithClang.ps1

# Debug build
.\BuildWithClang.ps1 -BuildType Debug

# 32-bit build
.\BuildWithClang.ps1 -Platform Win32

# Clean build
.\BuildWithClang.ps1 -Clean

# Install to specific location
.\BuildWithClang.ps1 -InstallPrefix "C:\Program Files\Capsicain"

# Full options
.\BuildWithClang.ps1 -BuildType <Debug|Release|RelWithDebInfo|MinSizeRel> -Platform <x64|Win32> -Clean -InstallPrefix <path> -RunTests -Verbose
```

## Alternative Build Methods

### Using the Original Build Script

The original `Build.ps1` script still works and can auto-detect your compiler:

```powershell
# Build with auto-detected compiler (prefers Clang if available)
.\Build.ps1

# Specify a compiler
.\Build.ps1 -Compiler clang
.\Build.ps1 -Compiler mingw
.\Build.ps1 -Compiler msvc

# Full options
.\Build.ps1 -BuildType <Debug|Release> -Platform <x64|Win32> -Compiler <clang|mingw|msvc> -Clean -Verbose
```

### Manual CMake Usage

You can also use CMake directly:

```bash
# Configure with Clang and Ninja (recommended)
mkdir build-clang
cd build-clang
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang ..
ninja

# Configure with MinGW
mkdir build-mingw
cd build-mingw
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make

# Configure with Visual Studio Build Tools
mkdir build-msvc
cd build-msvc
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

## Interception Library Options

By default, Capsicain uses dynamic linking with `interception.dll`. This means the DLL must be present alongside the executable for the program to run. The build scripts handle this automatically by copying `interception.dll` to the output directory.

### Dynamic vs Static Linking

The build system supports two ways to link with the interception library:

1. **Dynamic Linking** (default): Uses `interception.dll` at runtime
   - Pros: Smaller executable, easier to update the DLL independently
   - Cons: The DLL must be distributed with the executable

2. **Static Linking**: Links the library directly into the executable
   - Pros: Single-file distribution, no external DLL dependencies
   - Cons: Larger executable size

### Controlling Linking Method

To control how interception is linked, use the `-StaticLinkInterception` flag with the build scripts:

```powershell
# Use dynamic linking (default behavior)
.\Build.ps1 -BuildType Release -Platform x64

# Use static linking
.\Build.ps1 -BuildType Release -Platform x64 -StaticLinkInterception
```

### Troubleshooting DLL Issues

If you get errors about missing `interception.dll`:

1. Check that `interception.dll` exists in the `capsicain` directory
2. Verify that the DLL was copied to the output directory during build
3. For manual builds, copy `interception.dll` from the `capsicain` directory to the location of your executable

For MinGW/Clang builds using dynamic linking, the build system automatically creates an import library from the DLL using `gendef` and `dlltool`.

## CI/CD with GitHub Actions

The project includes a GitHub Actions workflow that builds Capsicain using Clang on Windows without Visual Studio dependencies. This workflow:

1. Sets up the LLVM toolchain, Ninja, and CMake
2. Builds the project with Clang
3. Creates release artifacts
4. Automatically attaches artifacts to GitHub Releases

See `.github/workflows/build-with-clang.yml` for the complete workflow definition.

## Troubleshooting

### Checking Tool Installation

Verify your tools are correctly installed:

```powershell
clang++ --version
cmake --version
ninja --version
```

### Interception Library Issues

If you get errors related to the interception library:

1. Make sure you have the interception.dll and .lib files in the capsicain folder
2. For MinGW/Clang, ensure you have `gendef` and `dlltool` in your PATH to create an import library

### Resource (.rc) Files

If you encounter issues with compiling the Windows resource (.rc) files:

1. For Clang: Make sure `llvm-rc` is in your PATH or fallback to `windres`
2. For MinGW: Ensure `windres` is in your PATH
3. For MSVC: No additional tools are needed

## Output Locations

Build outputs are placed in directories based on the compiler and build type:

- **Clang**: `build-clang-<Platform>-<BuildType>/capsicain.exe`
- **MinGW**: `build-<Platform>-<BuildType>/capsicain.exe`
- **MSVC**: `build-<Platform>-<BuildType>/<BuildType>/capsicain.exe`

## Building in CI Environments

The project is configured to work in headless CI environments like GitHub Actions. The toolchain only depends on open-source components that can be easily installed via package managers or direct downloads, making it ideal for automated builds.