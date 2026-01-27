## Improvement: Split Monolithic capsicain.cpp into Modules

### Problem
`capsicain.cpp` is 2919 lines long, making it difficult to:
- Navigate and understand the code
- Test individual components
- Maintain and extend functionality
- Collaborate with multiple developers

### Proposed Solution
Split into focused modules following Single Responsibility Principle:

```
capsicain/
├── core/
│   ├── Application.cpp/.h      # main(), initialization, cleanup
│   ├── EventLoop.cpp/.h        # Core keyboard event loop
│   └── KeyProcessor.cpp/.h     # Key processing logic
├── input/
│   ├── InterceptionHandler.cpp/.h  # Interception library wrapper
│   └── MouseHandler.cpp/.h     # Mouse event handling
├── config/
│   ├── ConfigParser.cpp/.h     # INI parsing (move from configUtils)
│   └── Settings.cpp/.h         # Global/Options structs
├── ui/
│   ├── Console.cpp/.h          # Console window management
│   └── TrayIcon.cpp/.h         # System tray functionality
└── integration/
    └── AutoHotkey.cpp/.h       # AHK DLL integration
```

### Implementation Steps
1. [ ] Create directory structure
2. [ ] Extract Application class with initialization logic
3. [ ] Extract EventLoop class encapsulating the main while loop
4. [ ] Move keyboard processing functions to KeyProcessor
5. [ ] Create InterceptionHandler wrapper class
6. [ ] Move console functions to Console class
7. [ ] Move AHK functions to AutoHotkey class
8. [ ] Update CMakeLists.txt with new files
9. [ ] Update include paths
10. [ ] Verify build and functionality

### Acceptance Criteria
- [ ] No single file exceeds 500 lines
- [ ] Each file has a single, clear responsibility
- [ ] All existing functionality preserved
- [ ] Build succeeds without warnings
- [ ] All ESC commands work correctly

### Files Affected
- capsicain/capsicain.cpp (primary)
- CMakeLists.txt
