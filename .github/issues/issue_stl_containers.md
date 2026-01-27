## Learning Task: STL Containers

### Objective
Learn how Standard Template Library containers are used throughout the codebase.

### Containers Used in This Project

| Container | Usage Example | File |
|-----------|---------------|------|
| `std::vector` | Key event sequences, config lines | capsicain.cpp |
| `std::map` | Combo mappings, device tracking | capsicain.cpp |
| `std::set` | Tracking held keys | capsicain.cpp |
| `std::string` | String manipulation | utils.cpp |

### What You'll Learn
1. Vector operations (push_back, insert, erase, iteration)
2. Map key-value storage and lookup
3. Set for unique element storage
4. Iterator patterns and range-based for loops

### Key Examples

```cpp
// Vector usage
vector<VKeyEvent> keyEventSequence;
keyEventSequence.push_back({ scancode, true });

// Map usage
map<string, vector<ModifierCombo>> modCombos;
for (auto& [key, combos] : modCombos) { ... }

// Set usage
set<int> holdKeys[VK_MAX];
holdKeys[i].clear();
```

### Tasks
- [ ] Study vector usage in GlobalState.recordedMacros
- [ ] Understand map usage in AllMaps.modCombos
- [ ] Trace how sanitizedIniContent vector is populated
- [ ] Find all iterator patterns in the codebase
- [ ] Understand when to use vector vs set vs map

### Related Files
- capsicain/capsicain.cpp
- capsicain/configUtils.cpp
- capsicain/utils.cpp
