## Improvement: Add Unit Testing Framework

### Problem
The codebase has no automated tests:
- Changes can break existing functionality
- Refactoring is risky
- No documentation of expected behavior
- No regression testing

### Proposed Solution
Add Google Test (gtest) for unit testing.

### Project Structure
```
capsicain/
├── src/              # Move source files here
├── include/          # Move headers here
├── tests/
│   ├── CMakeLists.txt
│   ├── test_main.cpp
│   ├── test_modifiers.cpp
│   ├── test_scancodes.cpp
│   ├── test_utils.cpp
│   ├── test_config_parser.cpp
│   └── test_key_processing.cpp
└── CMakeLists.txt    # Updated to include tests
```

### CMakeLists.txt Addition
```cmake
# Enable testing
enable_testing()

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Add test executable
add_executable(capsicain_tests
    tests/test_main.cpp
    tests/test_utils.cpp
    tests/test_modifiers.cpp
)
target_link_libraries(capsicain_tests GTest::gtest_main)

# Register tests
include(GoogleTest)
gtest_discover_tests(capsicain_tests)
```

### Example Test File
```cpp
// tests/test_utils.cpp
#include <gtest/gtest.h>
#include "utils.h"

TEST(StringUtils, StartsWith) {
    EXPECT_TRUE(stringStartsWith("hello world", "hello"));
    EXPECT_FALSE(stringStartsWith("hello world", "world"));
    EXPECT_TRUE(stringStartsWith("", ""));
}

TEST(StringUtils, ToLower) {
    EXPECT_EQ(stringToLower("HELLO"), "hello");
    EXPECT_EQ(stringToLower("HeLLo WoRLd"), "hello world");
}

TEST(Modifiers, BitmaskForVcode) {
    EXPECT_EQ(getModifierBitmaskForVcode(SC_LSHIFT), BITMASK_LSHIFT);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_LCTRL), BITMASK_LCTRL);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_A), 0);  // Not a modifier
}
```

### Implementation Steps
1. [ ] Create tests/ directory
2. [ ] Update CMakeLists.txt with FetchContent for gtest
3. [ ] Create test_main.cpp
4. [ ] Write tests for utils.cpp functions
5. [ ] Write tests for modifiers.cpp functions
6. [ ] Write tests for config parsing
7. [ ] Set up CI to run tests on push
8. [ ] Add code coverage reporting

### What to Test First
1. String utility functions (pure functions, easy to test)
2. Modifier bitmask operations
3. Scan code lookups
4. INI parsing (with test fixtures)
5. Key event conversion

### Acceptance Criteria
- [ ] Test framework integrated
- [ ] At least 10 test cases
- [ ] Tests run via CMake/CTest
- [ ] All tests pass
- [ ] README updated with test instructions

### Files Affected
- CMakeLists.txt
- New directory: tests/
