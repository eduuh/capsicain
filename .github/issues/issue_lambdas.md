## Learning Task: Lambda Expressions

### Objective
Learn how lambda expressions provide inline function definitions.

### Location
- **File:** capsicain/capsicain.cpp (processCombos function, ~line 1065)

### What You'll Learn
1. Lambda syntax and structure
2. Capture clauses (capturing by reference)
3. Parameter lists
4. Using lambdas with STL algorithms
5. Default parameter values in lambdas

### Key Example
```cpp
void processCombos()
{
    // Lambda with capture by reference and default parameter
    auto process = [](vector<ModifierCombo> &combos, bool clearTapped = false) {
        for (ModifierCombo modcombo : combos)
        {
            if (modcombo.vkey == loopState.vcode && ...)
            {
                loopState.resultingVKeyEventSequence = modcombo.keyEventSequence;
                if (clearTapped)
                    modifierState.modifierTapped = 0;
                break;
            }
        }
    };

    // Using the lambda
    if (loopState.isDownstroke) {
        process(allMaps.modCombos[INI_TAG_COMBOS], true);
        if (loopState.repeat)
            process(allMaps.modCombos[INI_TAG_REPEATCOMBOS]);
    }
}
```

### Lambda Syntax Breakdown
```cpp
auto lambda = [capture](parameters) -> return_type { body };

// [] - capture nothing
// [&] - capture all by reference
// [=] - capture all by value
// [&x, y] - x by reference, y by value
```

### Tasks
- [ ] Find the lambda in processCombos()
- [ ] Understand why it captures by reference
- [ ] Trace how the lambda is called multiple times
- [ ] Identify other potential lambda uses in the codebase
- [ ] Create a test program with different capture modes

### Related Files
- capsicain/capsicain.cpp
