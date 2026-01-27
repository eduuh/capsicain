## Improvement: Replace C-Style Arrays with std::array

### Problem
C-style arrays are used with manual initialization:

```cpp
int rewiremap[REWIRE_ROWS][REWIRE_COLS] = { }; // MUST initialize manually to -1!
int alphamap[MAX_VCODES] = { };                // MUST initialize manually!
bool keysDownSent[256] = { false };
```

Issues:
- No bounds checking
- Decay to pointers when passed to functions
- Manual size tracking required
- Comments warning about manual initialization

### Proposed Solution
Use `std::array` (C++11):

```cpp
#include <array>

// Type-safe, knows its own size
std::array<std::array<int, REWIRE_COLS>, REWIRE_ROWS> rewiremap;
std::array<int, MAX_VCODES> alphamap;
std::array<bool, 256> keysDownSent{};  // Zero-initialized

// Benefits:
// - .size() method
// - .fill() for initialization
// - .at() for bounds-checked access
// - Works with range-based for
// - Can be passed by reference without decay
```

### Initialization
```cpp
// Old
for (int r = 0; r < REWIRE_ROWS; r++)
    for (int c = 0; c < REWIRE_COLS; c++)
        allMaps.rewiremap[r][c] = -1;

// New
for (auto& row : allMaps.rewiremap)
    row.fill(-1);

// Or with algorithms
std::ranges::for_each(allMaps.rewiremap, [](auto& row) { row.fill(-1); });
```

### Specific Changes

| Variable | Old | New |
|----------|-----|-----|
| `rewiremap` | `int[ROWS][COLS]` | `std::array<std::array<int, COLS>, ROWS>` |
| `alphamap` | `int[MAX_VCODES]` | `std::array<int, MAX_VCODES>` |
| `keysDownSent` | `bool[256]` | `std::array<bool, 256>` |
| `keysDownTempReleased` | `bool[256]` | `std::array<bool, 256>` |

### Implementation Steps
1. [ ] Include `<array>` where needed
2. [ ] Replace array declarations with std::array
3. [ ] Update initialization code to use .fill()
4. [ ] Replace sizeof calculations with .size()
5. [ ] Consider .at() for debug builds (bounds checking)
6. [ ] Update function signatures to take std::array references

### Acceptance Criteria
- [ ] No C-style arrays for fixed-size buffers
- [ ] Clean initialization without warnings
- [ ] Build succeeds
- [ ] Performance equivalent (std::array has zero overhead)

### Files Affected
- capsicain/capsicain.cpp (AllMaps struct, GlobalState struct)
- capsicain/capsicain.h
