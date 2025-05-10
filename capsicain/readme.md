# Capsicain

Capsicain is a powerful keyboard remapping and macro tool for Windows, designed for advanced users and tinkerers. It supports custom layouts, macros, and device-specific remapping.

## Prerequisites

- **Windows 10/11**
- **Visual Studio 2019 or newer** (with "Desktop development with C++" workload)
- **CMake** (https://cmake.org/download/)
- **PowerShell** (comes with Windows)

## Quick Build Instructions

1. **Open a x64 Native Tools Command Prompt for VS 2019/2022** (or similar for your Visual Studio version).
2. **Clone the repository** (if you haven't already):
   ```sh
   git clone https://github.com/yourusername/capsicain.git
   cd capsicain
   ```
3. **Build using the provided PowerShell script:**
   ```powershell
   ./Build.ps1 -BuildType Debug
   ```
   - For a Release build, use `-BuildType Release`.
   - The script will configure and build using CMake and MSVC.

4. **Find the output:**
   - Debug build: `capsicain/build/cmake-x64-Debug/DEBUG/`
   - Release build: `capsicain/build/cmake-x64-Release/RELEASE/`
   - The folder will contain `capsicain.exe`, `capsicain.ini`, `interception.dll`, and `AutoHotKey64.dll`.

## Manual CMake Build

If you prefer to use CMake directly:

```sh
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Debug
```

## Running Capsicain

- Make sure `capsicain.ini`, `interception.dll`, and `AutoHotKey64.dll` are in the same folder as `capsicain.exe`.
- Run `capsicain.exe` from the output directory.
- Edit `capsicain.ini` to customize your keyboard layout and macros.

## Troubleshooting

- **Missing DLLs:** Ensure `interception.dll` and `AutoHotKey64.dll` are present in the output directory.
- **Build errors:** Make sure you have the correct Visual Studio version and CMake installed.
- **Debugging:** Use the Debug build for breakpoints and symbol support in Visual Studio or VS Code.

## License

See [LICENSE](../LICENSE) for details.
