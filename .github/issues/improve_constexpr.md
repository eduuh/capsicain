## Improvement: Replace #define with constexpr

### Problem
The codebase uses C-style `#define` macros for constants:

```cpp
#define VERSION "96"
#define MAX_VCODES 0x140
#define MAX_MACRO_LENGTH 200
#define DISABLED_CONFIG_NUMBER 0
```

Issues with #define:
- No type safety
- No scope (pollutes global namespace)
- Cannot be debugged
- No respect for namespaces

### Proposed Solution
Use modern C++ `constexpr` and `const`:

```cpp
// constants.h - Modern approach
namespace capsicain {
    constexpr std::string_view VERSION = "96";
    constexpr size_t MAX_VCODES = 0x140;
    constexpr size_t MAX_MACRO_LENGTH = 200;
    constexpr int DISABLED_CONFIG_NUMBER = 0;
    constexpr std::string_view DISABLED_CONFIG_NAME = 
        "Capsicain disabled...";
}
```

### Specific Changes

| Old | New |
|-----|-----|
| `#define VERSION "96"` | `constexpr std::string_view VERSION = "96";` |
| `#define MAX_VCODES 0x140` | `constexpr size_t MAX_VCODES = 0x140;` |
| `#define DEFAULT_DELAY_FOR_KEY_SEQUENCE_MS 5` | `constexpr int DEFAULT_DELAY_FOR_KEY_SEQUENCE_MS = 5;` |

### Implementation Steps
1. [ ] Create `capsicain` namespace in constants.h
2. [ ] Convert all numeric `#define` to `constexpr`
3. [ ] Convert string `#define` to `constexpr std::string_view`
4. [ ] Update all usages to use namespace prefix or `using`
5. [ ] Remove old `#define` statements
6. [ ] Verify build and functionality

### Note on IFDEBUG/IFTRACE Macros
Keep macros that control conditional compilation:
```cpp
#define IFDEBUG if(options.debug)  // Keep as macro
```
These cannot be replaced with constexpr as they control code generation.

### Acceptance Criteria
- [ ] No `#define` for constants (except conditional compilation)
- [ ] All constants are type-safe
- [ ] Build succeeds
- [ ] No runtime behavior changes

### Files Affected
- capsicain/constants.h (primary)
- All files that use these constants
