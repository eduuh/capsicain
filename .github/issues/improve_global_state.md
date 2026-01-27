## Improvement: Eliminate Global State with Encapsulation

### Problem
The codebase uses multiple global structs that create hidden dependencies:
- `globals` - INI settings
- `options` - Runtime options
- `globalState` - Application state
- `modifierState` - Modifier tracking
- `loopState` - Per-key state
- `interceptionState` - Interception context
- `allMaps` - All mappings
- `profiler` - Timing data

This leads to:
- Difficult testing (can't isolate components)
- Hidden dependencies between functions
- Race conditions if threading is added
- Difficult to reason about state changes

### Proposed Solution
Encapsulate state in a singleton or use dependency injection:

```cpp
// Option 1: Singleton Application class
class CapsicainApp {
private:
    CapsicainApp() = default;
    
    Settings settings_;
    RuntimeOptions options_;
    AppState state_;
    ModifierTracker modifiers_;
    KeyMappings mappings_;
    InterceptionContext interception_;
    
public:
    static CapsicainApp& instance();
    
    Settings& settings() { return settings_; }
    RuntimeOptions& options() { return options_; }
    // ...
};

// Option 2: Context object passed to functions
struct CapsicainContext {
    Settings settings;
    RuntimeOptions options;
    // ...
};

void processKey(CapsicainContext& ctx, VKeyEvent event);
```

### Implementation Steps
1. [ ] Create CapsicainApp class with private constructor
2. [ ] Move global structs as private members
3. [ ] Add accessor methods
4. [ ] Update all functions to use CapsicainApp::instance()
5. [ ] Remove global variables
6. [ ] Add reset() method for testing

### Acceptance Criteria
- [ ] No global variables remain (except the singleton)
- [ ] All state accessible through CapsicainApp
- [ ] Existing functionality preserved
- [ ] Code compiles without errors

### Files Affected
- capsicain/capsicain.cpp
- capsicain/capsicain.h
- All files that reference global state
