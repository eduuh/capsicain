## Improvement: Use Structured Bindings (C++17)

### Problem
Current code uses verbose patterns for accessing map entries:

```cpp
// Current verbose style
for (auto& kv : allMaps.modCombos) {
    string key = kv.first;
    vector<ModifierCombo>& combos = kv.second;
    // use key and combos
}
```

### Proposed Solution
Use C++17 structured bindings for cleaner code:

```cpp
// Modern C++17 style
for (auto& [key, combos] : allMaps.modCombos) {
    // key and combos available directly
}
```

### Examples to Update

#### Map Iteration
```cpp
// Before
for (auto& kv : allMaps.devices) {
    int deviceId = kv.first;
    Device& device = kv.second;
}

// After
for (auto& [deviceId, device] : allMaps.devices) {
    // Use deviceId and device directly
}
```

#### Pair Returns
```cpp
// Before
auto result = map.insert({key, value});
auto it = result.first;
bool inserted = result.second;

// After
auto [it, inserted] = map.insert({key, value});
```

#### Multiple Return Values
```cpp
// Before
tuple<int, int, int> getValues();
auto values = getValues();
int a = get<0>(values);
int b = get<1>(values);

// After
auto [a, b, c] = getValues();
```

### Implementation Steps
1. [ ] Find all map iterations with .first/.second
2. [ ] Convert to structured bindings
3. [ ] Find tuple/pair unpacking
4. [ ] Convert to structured bindings
5. [ ] Verify build with C++17 flag

### Files to Search
```bash
grep -r "\.first" capsicain/
grep -r "\.second" capsicain/
grep -r "get<" capsicain/
```

### Acceptance Criteria
- [ ] No .first/.second in range-based for loops
- [ ] Cleaner, more readable code
- [ ] Build succeeds with C++17
- [ ] No functionality changes

### Files Affected
- capsicain/capsicain.cpp
- capsicain/configUtils.cpp
- Any file iterating over maps
