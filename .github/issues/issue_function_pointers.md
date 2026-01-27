## Learning Task: Function Pointers

### Objective
Learn how function pointers are used for dynamic DLL loading.

### Location
- **File:** capsicain/capsicain.cpp (lines 23-40)

### What You'll Learn
1. Defining function pointer types with typedef
2. Loading DLLs dynamically with LoadLibrary
3. Getting function addresses with GetProcAddress
4. Calling functions through pointers

### Key Code
```cpp
// Define function pointer types
typedef int (*AHKTHREAD)(const wchar_t* aScript, ...);
typedef int (*AHKREADY)(int threadid);

// Struct to hold function pointers
struct Ahk {
    HMODULE handle;
    AHKTHREAD thread;
    AHKREADY ready;
    // ...
} ahk;

// Loading the DLL and getting function addresses
ahk.handle = LoadLibrary(TEXT("AutoHotkey64.dll"));
ahk.thread = (AHKTHREAD)GetProcAddress(ahk.handle, "NewThread");
```

### Tasks
- [ ] Study the typedef declarations for AHK functions
- [ ] Understand the Ahk struct organization
- [ ] Trace loadAHK() function step by step
- [ ] Understand error handling when DLL is missing
- [ ] Learn about unloadAHK() cleanup

### Why This Matters
- Plugin architecture patterns
- Runtime extensibility
- Windows API integration
- Resource management (FreeLibrary)

### Related Files
- capsicain/capsicain.cpp (loadAHK, unloadAHK functions)
