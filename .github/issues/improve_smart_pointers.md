## Improvement: Use Smart Pointers for Resource Management

### Problem
Manual resource management with raw pointers:

```cpp
// DLL handling
HMODULE handle;
// ... later must remember to call:
FreeLibrary(handle);

// Process handles
HANDLE hProc = OpenProcess(...);
// ... must remember to call:
CloseHandle(hProc);
```

Issues:
- Easy to forget cleanup (memory/resource leaks)
- Exception-unsafe (if exception thrown, cleanup skipped)
- Multiple return paths make cleanup complex

### Proposed Solution
Use RAII wrappers and smart pointers:

```cpp
// Custom deleter for HMODULE
struct HModuleDeleter {
    void operator()(HMODULE h) const {
        if (h) FreeLibrary(h);
    }
};
using UniqueHModule = std::unique_ptr<std::remove_pointer_t<HMODULE>, HModuleDeleter>;

// Custom deleter for HANDLE
struct HandleDeleter {
    void operator()(HANDLE h) const {
        if (h && h != INVALID_HANDLE_VALUE) CloseHandle(h);
    }
};
using UniqueHandle = std::unique_ptr<std::remove_pointer_t<HANDLE>, HandleDeleter>;

// Usage
UniqueHModule ahkDll(LoadLibrary(TEXT("AutoHotkey64.dll")));
if (!ahkDll) {
    // Handle error
}
// Automatically freed when ahkDll goes out of scope
```

### RAII Wrapper Class Alternative
```cpp
class ScopedHandle {
    HANDLE h_;
public:
    explicit ScopedHandle(HANDLE h = nullptr) : h_(h) {}
    ~ScopedHandle() { 
        if (h_ && h_ != INVALID_HANDLE_VALUE) 
            CloseHandle(h_); 
    }
    
    // Delete copy
    ScopedHandle(const ScopedHandle&) = delete;
    ScopedHandle& operator=(const ScopedHandle&) = delete;
    
    // Allow move
    ScopedHandle(ScopedHandle&& other) noexcept : h_(other.h_) {
        other.h_ = nullptr;
    }
    
    HANDLE get() const { return h_; }
    explicit operator bool() const { return h_ != nullptr; }
};
```

### Resources to Wrap
1. [ ] HMODULE (DLL handles)
2. [ ] HANDLE (process/thread handles)
3. [ ] HWND considerations (usually not owned)
4. [ ] InterceptionContext

### Implementation Steps
1. [ ] Create raii_handles.h with wrapper types
2. [ ] Replace raw HMODULE in Ahk struct
3. [ ] Replace raw HANDLE in process management
4. [ ] Test DLL loading/unloading
5. [ ] Test process management functions

### Acceptance Criteria
- [ ] No raw handle types that require manual cleanup
- [ ] All resources automatically cleaned up
- [ ] Exception-safe resource management
- [ ] Build succeeds

### Files Affected
- capsicain/capsicain.cpp (Ahk struct, process management)
- capsicain/utils.cpp (process functions)
- New file: capsicain/raii_handles.h
