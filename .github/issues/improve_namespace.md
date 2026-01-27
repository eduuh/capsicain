## Improvement: Remove 'using namespace std' from Global Scope

### Problem
The codebase has `using namespace std;` in the global scope of .cpp files:

```cpp
// capsicain.cpp
using namespace std;

// This can cause:
// 1. Name collisions with Windows API
// 2. Ambiguous function calls
// 3. Unexpected behavior when new std:: items added
// 4. Harder to identify what comes from std::
```

### Why It's Problematic
```cpp
using namespace std;

// Which 'find' is this? std::find? Windows API find?
auto result = find(container.begin(), container.end(), value);

// Future C++ versions might add std::count that conflicts
int count = 0;  // Works now, might conflict later
```

### Proposed Solution
Remove global `using namespace std;` and either:

1. **Use explicit std:: prefix** (recommended)
```cpp
std::vector<std::string> lines;
std::cout << "Hello" << std::endl;
```

2. **Use selective using declarations** (acceptable)
```cpp
using std::vector;
using std::string;
using std::cout;
using std::endl;
```

3. **Use using inside function scope** (acceptable)
```cpp
void someFunction() {
    using namespace std;  // Limited scope
    // ...
}
```

### Implementation Steps
1. [ ] Remove `using namespace std;` from capsicain.cpp
2. [ ] Add `std::` prefix to all standard library usages
3. [ ] Remove from utils.cpp
4. [ ] Remove from configUtils.cpp
5. [ ] Remove from modifiers.cpp
6. [ ] Remove from scancodes.cpp
7. [ ] Remove from traybar.cpp
8. [ ] Remove from led.cpp
9. [ ] Verify build succeeds

### Common Prefixes Needed
- `std::string`
- `std::vector`
- `std::map`
- `std::set`
- `std::cout`, `std::endl`
- `std::chrono::`
- `std::to_string()`

### Acceptance Criteria
- [ ] No `using namespace std;` in global scope
- [ ] All std library items explicitly prefixed
- [ ] Build succeeds without errors
- [ ] No functionality changes

### Files Affected
- capsicain/capsicain.cpp
- capsicain/utils.cpp
- capsicain/configUtils.cpp
- capsicain/modifiers.cpp
- capsicain/scancodes.cpp
- capsicain/traybar.cpp
- capsicain/led.cpp
