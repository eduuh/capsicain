# Capsicain Architecture Refactoring Plan

## Current State Analysis

### Problems with Current Architecture

1. **Monolithic Design** - [capsicain.cpp](../capsicain/capsicain.cpp) is ~3000 lines with all logic in one file
2. **Global Mutable State** - 8+ global structs scattered at file scope:
   - `globals`, `options`, `globalState`, `modifierState`, `loopState`
   - `allMaps`, `interceptionState`, `profiler`
3. **Tight Coupling** - Hardware interaction (Interception) mixed with business logic
4. **Hard to Test** - Functions depend on global state, not parameters
5. **No Interfaces** - Can't mock hardware layer for testing

### Current Code Flow

```
main()
├── initConsoleWindow()
├── interception_create_context()
├── parseIniGlobals()
├── switchConfig()
└── CORE LOOP
    ├── interception_receive()          # I/O
    ├── processOnOffKey()               # Logic
    ├── processMessyKeys()              # Logic
    ├── detectTapping()                 # Logic
    ├── processRewireScancodeToVirtualcode()  # Logic
    ├── processModifierState()          # Logic
    ├── processCombos()                 # Logic
    ├── processMapAlphaKeys()           # Logic
    └── sendResultingKeyOrSequence()    # I/O
```

---

## Proposed Architecture

### Layer Separation

```
┌─────────────────────────────────────────────────────────┐
│                     Application Layer                    │
│  main.cpp - Entry point, CLI, tray icon                 │
└─────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────┐
│                     Service Layer                        │
│  KeyProcessingService - Orchestrates the pipeline       │
│  ConfigService - Loads/switches configurations          │
│  MacroService - Records/plays macros                    │
└─────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────┐
│                     Domain Layer                         │
│  KeyProcessor - Core key transformation logic           │
│  ComboMatcher - Evaluates combo rules                   │
│  TapDetector - Detects tap/hold/slow-tap                │
│  ModifierTracker - Tracks modifier state                │
│  AlphaMapper - Character layout remapping               │
└─────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────┐
│                   Infrastructure Layer                   │
│  IInputDriver ←── InterceptionDriver                    │
│  IOutputDriver ←── InterceptionOutput                   │
│  IConfigParser ←── IniParser                            │
└─────────────────────────────────────────────────────────┘
```

### Proposed Directory Structure

```
capsicain/
├── src/
│   ├── main.cpp                    # Entry point only
│   │
│   ├── core/                       # Domain objects (no dependencies)
│   │   ├── KeyEvent.h              # VKeyEvent struct
│   │   ├── ScanCodes.h/.cpp        # Enum + lookup (exists)
│   │   ├── Modifiers.h/.cpp        # Bitmask logic (exists)
│   │   ├── Config.h                # Configuration data structures
│   │   └── Types.h                 # Common types (MOD, DEV, etc.)
│   │
│   ├── domain/                     # Business logic (pure functions)
│   │   ├── KeyProcessor.h/.cpp     # Main processing pipeline
│   │   ├── ComboMatcher.h/.cpp     # Rule matching engine
│   │   ├── TapDetector.h/.cpp      # Tap/hold detection
│   │   ├── ModifierTracker.h/.cpp  # Modifier state machine
│   │   ├── AlphaMapper.h/.cpp      # Alpha key remapping
│   │   └── RewireMapper.h/.cpp     # Rewire table logic
│   │
│   ├── services/                   # Application services
│   │   ├── KeyProcessingService.h/.cpp
│   │   ├── ConfigService.h/.cpp
│   │   └── MacroService.h/.cpp
│   │
│   ├── infrastructure/             # External interfaces
│   │   ├── IInputDriver.h          # Abstract input interface
│   │   ├── IOutputDriver.h         # Abstract output interface
│   │   ├── InterceptionDriver.h/.cpp
│   │   ├── IniParser.h/.cpp
│   │   └── TrayIcon.h/.cpp
│   │
│   └── utils/                      # Pure utility functions
│       ├── StringUtils.h/.cpp      # (exists in utils.cpp)
│       ├── TimeUtils.h/.cpp
│       └── ProcessUtils.h/.cpp
│
├── tests/
│   ├── unit/
│   │   ├── test_string_utils.cpp   # (exists)
│   │   ├── test_modifiers.cpp      # (exists)
│   │   ├── test_scancodes.cpp      # (exists)
│   │   ├── test_key_processor.cpp
│   │   ├── test_combo_matcher.cpp
│   │   ├── test_tap_detector.cpp
│   │   └── test_alpha_mapper.cpp
│   │
│   ├── integration/
│   │   └── test_full_pipeline.cpp
│   │
│   └── mocks/
│       ├── MockInputDriver.h
│       └── MockOutputDriver.h
│
└── include/                        # Public headers (if making a library)
```

---

## Implementation Phases

### Phase 1: Extract Pure Functions (Low Risk) ✓ Started
- [x] Add unit testing framework
- [ ] Extract string utilities to separate compilation unit
- [ ] Add tests for existing pure functions
- [ ] Extract `normalizeLine()`, `checkSyntax()` from configUtils

### Phase 2: Create Domain Objects
- [ ] Create `KeyEvent` struct (currently `VKeyEvent`)
- [ ] Create `ProcessingContext` to replace `loopState`
- [ ] Create `ModifierState` class with methods (not just struct)
- [ ] Create `Config` class to hold all config data

### Phase 3: Create Interfaces for I/O
- [ ] Define `IInputDriver` interface
- [ ] Define `IOutputDriver` interface  
- [ ] Wrap Interception in `InterceptionDriver` class
- [ ] Create `MockInputDriver` for testing

### Phase 4: Extract Processing Logic
- [ ] Extract `detectTapping()` → `TapDetector` class
- [ ] Extract `processModifierState()` → `ModifierTracker` class
- [ ] Extract `processCombos()` → `ComboMatcher` class
- [ ] Extract `processMapAlphaKeys()` → `AlphaMapper` class
- [ ] Extract `processRewireScancodeToVirtualcode()` → `RewireMapper` class

### Phase 5: Create Processing Pipeline
- [ ] Create `KeyProcessor` that composes all mappers
- [ ] Inject dependencies instead of using globals
- [ ] Create `KeyProcessingService` to orchestrate

### Phase 6: Refactor Main
- [ ] Slim down `main()` to just initialization
- [ ] Move core loop to `KeyProcessingService`
- [ ] Move config loading to `ConfigService`

---

## Key Interfaces

### IInputDriver
```cpp
class IInputDriver {
public:
    virtual ~IInputDriver() = default;
    virtual bool initialize() = 0;
    virtual std::optional<KeyEvent> waitForKey(int timeoutMs) = 0;
    virtual DeviceInfo getDeviceInfo(DeviceHandle device) = 0;
    virtual void shutdown() = 0;
};
```

### IOutputDriver
```cpp
class IOutputDriver {
public:
    virtual ~IOutputDriver() = default;
    virtual void sendKey(const KeyEvent& event) = 0;
    virtual void sendSequence(const std::vector<KeyEvent>& events) = 0;
};
```

### KeyProcessor
```cpp
class KeyProcessor {
public:
    KeyProcessor(
        RewireMapper& rewirer,
        ModifierTracker& modifiers,
        TapDetector& tapDetector,
        ComboMatcher& combos,
        AlphaMapper& alphaMapper
    );
    
    std::vector<KeyEvent> process(const KeyEvent& input, const Config& config);
};
```

---

## Migration Strategy

1. **Keep existing code working** - All changes are additive
2. **One function at a time** - Extract, test, integrate
3. **Feature flags** - New code can be toggled for comparison
4. **Dual running** - Run old and new in parallel during transition

---

## Next Steps

1. **Start with Phase 2** - Create `ProcessingContext` to replace globals
2. **Extract TapDetector** - Smallest, most isolated component
3. **Add integration tests** - Capture current behavior before refactoring
