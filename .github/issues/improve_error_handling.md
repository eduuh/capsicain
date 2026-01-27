## Improvement: Add Proper Error Handling Strategy

### Problem
Error handling is inconsistent across the codebase:

```cpp
// Sometimes: error() function + cout
error("Bad Rewire / key mapping: " + line);

// Sometimes: just cout
cout << endl << "ERROR: cannot find config";

// Sometimes: silent failure
if (!f.is_open())
    return false;  // Caller has no idea why
```

Issues:
- No consistent error reporting mechanism
- Errors mixed with debug output
- No error codes or categories
- Difficult to handle errors programmatically

### Proposed Solution

#### Option 1: Exception-Based (Recommended for C++)
```cpp
#include <stdexcept>

class CapsicainError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ConfigError : public CapsicainError {
public:
    using CapsicainError::CapsicainError;
};

class ParseError : public CapsicainError {
    int line_;
public:
    ParseError(const std::string& msg, int line) 
        : CapsicainError(msg), line_(line) {}
    int line() const { return line_; }
};

// Usage
if (!f.is_open()) {
    throw ConfigError("Cannot open capsicain.ini");
}
```

#### Option 2: Result Type (Modern, No Exceptions)
```cpp
#include <variant>
#include <string>

template<typename T>
using Result = std::variant<T, std::string>;  // Value or error message

Result<std::vector<std::string>> readIniFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        return "Cannot open file: " + path;
    }
    // ... read file
    return lines;  // Success
}

// Usage
auto result = readIniFile("capsicain.ini");
if (auto* error = std::get_if<std::string>(&result)) {
    std::cerr << "Error: " << *error << std::endl;
} else {
    auto& lines = std::get<std::vector<std::string>>(result);
    // Process lines
}
```

### Implementation Steps
1. [ ] Define error hierarchy or Result type
2. [ ] Create error.h with error types
3. [ ] Update file operations to throw/return errors
4. [ ] Update parsing functions
5. [ ] Add try-catch in main() for top-level handling
6. [ ] Replace cout error messages with proper errors
7. [ ] Add error logging to file

### Centralized Logging
```cpp
enum class LogLevel { Debug, Info, Warning, Error };

void log(LogLevel level, const std::string& msg) {
    // Output to console with color
    // Append to error log
    // Consider timestamp
}
```

### Acceptance Criteria
- [ ] Consistent error handling strategy documented
- [ ] All errors go through single mechanism
- [ ] Errors include context (file, line, etc.)
- [ ] User-facing errors are clear and actionable

### Files Affected
- New file: capsicain/error.h
- capsicain/capsicain.cpp
- capsicain/configUtils.cpp
- capsicain/utils.cpp
