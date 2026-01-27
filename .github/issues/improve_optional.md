## Improvement: Use std::optional for Nullable Values

### Problem
The codebase uses magic values to represent "not set":

```cpp
int capsicainOnOffKey = -1;  // -1 means "not defined"
int tapAndHoldKey = -1;      // -1 means "no tap-and-hold active"
int recordingMacro = -1;     // -1 means "not recording"
```

Issues:
- Magic values are error-prone
- Semantic meaning is not clear from type
- Easy to forget to check for -1
- -1 might be a valid value in some contexts

### Proposed Solution
Use `std::optional` (C++17):

```cpp
#include <optional>

struct Globals {
    std::optional<int> capsicainOnOffKey;  // No magic value needed
    // ...
};

struct ModifierState {
    std::optional<int> tapAndHoldKey;
    // ...
};

// Usage
if (globals.capsicainOnOffKey.has_value()) {
    int key = globals.capsicainOnOffKey.value();
    // or: int key = *globals.capsicainOnOffKey;
}

// Or with value_or for defaults
int key = globals.capsicainOnOffKey.value_or(SC_ESCAPE);
```

### Specific Changes

| Variable | Old Type | New Type |
|----------|----------|----------|
| `capsicainOnOffKey` | `int` (-1 = undefined) | `std::optional<int>` |
| `tapAndHoldKey` | `int` (-1 = none) | `std::optional<int>` |
| `recordingMacro` | `int` (-1 = not recording) | `std::optional<int>` |

### Implementation Steps
1. [ ] Include `<optional>` header
2. [ ] Change variable declarations to use std::optional
3. [ ] Replace `-1` checks with `.has_value()`
4. [ ] Replace value access with `.value()` or `*`
5. [ ] Replace assignments of -1 with `std::nullopt`
6. [ ] Test all affected code paths

### Code Migration Example
```cpp
// Before
if (globals.capsicainOnOffKey != -1) {
    if (loopState.scancode == globals.capsicainOnOffKey) {
        // ...
    }
}

// After
if (globals.capsicainOnOffKey.has_value()) {
    if (loopState.scancode == *globals.capsicainOnOffKey) {
        // ...
    }
}
```

### Acceptance Criteria
- [ ] No magic value -1 for "undefined"
- [ ] All optional values use std::optional
- [ ] Build succeeds
- [ ] All tests pass

### Files Affected
- capsicain/capsicain.cpp
- capsicain/capsicain.h
