## Improvement: Use std::string_view for Read-Only Strings

### Problem
String functions copy strings unnecessarily:

```cpp
// Current - copies string on every call
bool stringStartsWith(string haystack, string needle);
std::string stringToLower(std::string str);
```

Each call allocates memory and copies the string content, even when only reading.

### Proposed Solution
Use `std::string_view` (C++17) for read-only access:

```cpp
#include <string_view>

// New - no copying, just a view into existing string
bool stringStartsWith(std::string_view haystack, std::string_view needle);

// For functions that return modified strings, keep std::string
std::string stringToLower(std::string_view str);
```

### Benefits
- Zero-copy for read-only operations
- Works with string literals, std::string, and char*
- Significant performance improvement for string-heavy code
- No heap allocation for temporary views

### Functions to Update in utils.h

| Function | Current | Proposed |
|----------|---------|----------|
| `stringStartsWith` | `string, string` | `string_view, string_view` |
| `stringGetLastToken` | `string` | `string_view` |
| `stringGetRestBehindFirstToken` | `string` | `string_view` → `string` |
| `stringCopyFirstToken` | `string` | `string_view` → `string` |
| `stringToLower` | `string` | `string_view` → `string` |
| `stringToUpper` | `string` | `string_view` → `string` |

### Implementation Steps
1. [ ] Include `<string_view>` in utils.h
2. [ ] Update function signatures for read-only params
3. [ ] Update implementations
4. [ ] Update call sites if needed
5. [ ] Benchmark string-heavy operations

### Important Considerations
- `string_view` does NOT own data - don't return views to local strings
- Views can become invalid if underlying string is modified
- Use `string_view` for parameters, `string` for return values

### Code Example
```cpp
// Before
bool stringStartsWith(string haystack, string needle) {
    return haystack.compare(0, needle.length(), needle) == 0;
}

// After
bool stringStartsWith(std::string_view haystack, std::string_view needle) {
    return haystack.substr(0, needle.length()) == needle;
    // Or: return haystack.starts_with(needle);  // C++20
}
```

### Acceptance Criteria
- [ ] All read-only string params use string_view
- [ ] No dangling string_view references
- [ ] Build succeeds
- [ ] Functionality preserved

### Files Affected
- capsicain/utils.h
- capsicain/utils.cpp
- All callers of string functions
