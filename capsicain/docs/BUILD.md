# Building Capsicain

Capsicain is built using Visual Studio (MSVC) C++ tools and CMake on Windows. Only dynamic linking with interception.dll is supported.

## Prerequisites

- Visual Studio 2019 or newer (with "Desktop development with C++" workload)
- CMake (https://cmake.org/download/)

## Quick Build Instructions

1. Open a "x64 Native Tools Command Prompt for VS 2019/2022" (or similar for your Visual Studio version).
2. Navigate to the project root directory.
3. Run the build script:
   ```powershell
   ./Build.ps1
   ```
   By default, this builds a Release x64 version. For Debug or Win32, use:
   ```powershell
   ./Build.ps1 -BuildType Debug -Platform Win32
   ```

## Manual Build (CMake)

If you prefer to run CMake manually:

1. Create and enter a build directory:
   ```cmd
   mkdir build
   cd build
   ```
2. Configure the project:
   ```cmd
   cmake -G "Visual Studio 17 2022" -A x64 ..
   ```
3. Build the project:
   ```cmd
   cmake --build . --config Release
   ```

## Output

- Executable: `capsicain/build/cmake-x64-Release/Release/capsicain.exe`
- DLL: `capsicain/build/cmake-x64-Release/Release/interception.dll` (copied automatically)
- INI files: `capsicain/build/cmake-x64-Release/Release/`

## Notes

- Only Visual Studio/MSVC is supported. Other compilers are not supported.
- Only dynamic linking with interception.dll is supported. The DLL must be present with the executable.
- The build script also supports packaging and release options. Run `./Build.ps1 -?` for details.

## Running Unit Tests

Capsicain includes a unit test suite using Google Test. Tests are automatically downloaded and built via CMake's FetchContent.

### Building Tests

Tests are enabled by default. To configure and build:

```powershell
# Configure (downloads Google Test automatically)
cmake -S . -B capsicain/build/cmake-x64-Debug -G "Visual Studio 17 2022" -A x64

# Build tests
cmake --build capsicain/build/cmake-x64-Debug --target capsicain_tests --config Debug
```

### Running Tests

Use CTest to run all tests:

```powershell
cd capsicain/build/cmake-x64-Debug
ctest -C Debug --output-on-failure
```

Or run the test executable directly:

```powershell
.\capsicain\build\cmake-x64-Debug\DEBUG\capsicain_tests.exe
```

### Test Coverage

The test suite covers:
- **String utilities** (`test_utils.cpp`): 50 tests for string manipulation functions
- **Modifier operations** (`test_modifiers.cpp`): 21 tests for bitmask and modifier type checking
- **Scan codes** (`test_scancodes.cpp`): 19 tests for scan code lookups and enum values

### Disabling Tests

To build without tests:

```powershell
cmake -S . -B build -DBUILD_TESTING=OFF
```