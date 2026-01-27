## Improvement: Use enum class Instead of Plain enum

### Problem
Plain enums have issues:
- Values leak into enclosing scope
- Implicit conversion to int
- No type safety between different enums
- Name collisions possible

Current code:
```cpp
enum ScanCode {
    SC_NOP = 0x00,
    SC_ESCAPE = 0x01,
    // ...
};

// Problem: SC_ESCAPE is in global scope
// Problem: Can accidentally compare ScanCode with VirtualCode
```

### Proposed Solution
Use C++11 scoped enums (`enum class`):

```cpp
enum class ScanCode : uint16_t {
    NOP = 0x00,
    ESCAPE = 0x01,
    KEY_1 = 0x02,
    // ...
};

enum class VirtualCode : uint16_t {
    MOUSE_LEFT = 0xF1,
    CPS_TEMPRELEASEKEYS = 0x101,
    // ...
};

// Usage
ScanCode code = ScanCode::ESCAPE;
if (code == ScanCode::ESCAPE) { ... }

// Won't compile - type safe!
// if (scanCode == virtualCode) { ... }
```

### Benefits
- Type safety - can't mix ScanCode with VirtualCode
- Scoped names - no namespace pollution
- Explicit underlying type
- Forward declarable

### Implementation Steps
1. [ ] Change `enum ScanCode` to `enum class ScanCode : uint16_t`
2. [ ] Change `enum VirtualCode` to `enum class VirtualCode : uint16_t`
3. [ ] Update all usages to use scope operator (SC_ESCAPE → ScanCode::ESCAPE)
4. [ ] Add explicit casts where int conversion is needed
5. [ ] Update switch statements
6. [ ] Update array indexing (requires static_cast)

### Handling Array Indexing
```cpp
// Old
PRETTY_VK_LABELS[SC_ESCAPE] = "ESC";

// New - helper function
template<typename E>
constexpr auto to_underlying(E e) {
    return static_cast<std::underlying_type_t<E>>(e);
}

PRETTY_VK_LABELS[to_underlying(ScanCode::ESCAPE)] = "ESC";
```

### Acceptance Criteria
- [ ] All enums converted to enum class
- [ ] No implicit int conversions
- [ ] Build succeeds without warnings
- [ ] All functionality preserved

### Files Affected
- capsicain/scancodes.h
- capsicain/scancodes.cpp
- All files using scan codes
