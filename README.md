# Capsicain

Advanced keyboard remapping tool for Windows using low-level driver interception.

## Features

- **Low-level key interception** - Captures keys before Windows processes them
- **Custom keyboard layouts** - QWERTY, Dvorak, Colemak, Workman, etc.
- **Tap/Hold modifiers** - Different actions for tap vs hold (tap Caps for Esc, hold for Ctrl)
- **Key combos** - Layer switching with modifier combinations
- **Tap dance** - Multiple taps trigger different actions
- **Device-specific configs** - Different mappings per keyboard
- **Macro recording** - Record and replay key sequences
- **Leader keys** - Vim-style leader key sequences
- **Alpha remapping** - Full keyboard layout customization

## Prerequisites

- **Windows 10/11**
- **Visual Studio 2019+** with "Desktop development with C++"
- **CMake 3.15+**
- **Interception driver** must be installed

## Quick Start

### Build

```powershell
# Using build script
./Build.ps1

# Or manually with CMake
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### Run

```powershell
cd build/Release
./capsicain.exe
```

Edit `capsicain.ini` to customize your key mappings.

## Project Structure

```
capsicain/
├── src/
│   ├── app/           # Application layer
│   ├── commands/      # Command handling
│   ├── ui/            # Console UI
│   ├── core/          # Core types
│   ├── domain/        # Domain logic (pure functions)
│   ├── interfaces/    # Interfaces
│   ├── legacy/        # Legacy monolithic code (being refactored)
│   ├── platform/      # Windows/Interception specific
│   └── assets/        # Config files, DLLs
├── tests/             # Unit tests (196 tests)
└── build/             # Build output
```

## Testing

```powershell
cd build
ctest -C Debug --output-on-failure
```

All 196 unit tests must pass.

## Architecture

The codebase follows a domain-driven design with clean separation of concerns:

- **Domain layer** - Pure business logic (TapDetector, ModifierTracker, ComboMatcher, KeyMapper)
- **Application layer** - Orchestration and I/O
- **Platform layer** - Windows/Interception driver interface

All domain components use modern C++17 with:
- `noexcept` specifications for optimization
- `constexpr` for compile-time evaluation
- Explicit Rule of 5 for all classes
- No manual memory management
- Comprehensive unit test coverage

## Key Mappings

See `src/assets/capsicain.ini` for configuration examples:
- `REWIRE` - Simple key remapping
- `COMBOS` - Modifier combinations
- `ALPHA_FROM/ALPHA_TO` - Layout remapping
- `TAP/TAPHOLD` - Dual-function keys

## Commands

- **ESC+X** - Exit
- **ESC+R** - Reload config
- **ESC+0-9** - Switch layers
- **ESC+H** - Help

## License

See LICENSE file for details.

## Credits

Fork of original Capsicain by cajhin.
