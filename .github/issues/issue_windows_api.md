## Learning Task: Windows API Integration

### Objective
Learn how to integrate with Windows APIs for system-level functionality.

### Location
- **Files:** capsicain/capsicain.cpp, capsicain/traybar.cpp, capsicain/utils.cpp

### What You'll Learn
1. Console window manipulation
2. System tray (notification area) icons
3. Process management (finding, starting, killing)
4. Clipboard operations
5. Mutex for single-instance applications

### Key APIs Covered

| API | Purpose | Location |
|-----|---------|----------|
| CreateMutex | Single instance check | capsicain.cpp:1281 |
| SetConsoleCtrlHandler | Disable Ctrl+C | capsicain.cpp:1291 |
| Shell_NotifyIcon | System tray | traybar.cpp |
| ShellExecuteEx | Launch programs | capsicain.cpp |
| GlobalAlloc/SetClipboardData | Clipboard | utils.cpp |
| CreateToolhelp32Snapshot | Process enumeration | utils.cpp |

### Tasks
- [ ] Study initConsoleWindow() function
- [ ] Understand ShowInTraybar() implementation
- [ ] Trace FindProcessId() for process enumeration
- [ ] Learn copyToClipBoard() implementation
- [ ] Understand raise_process_priority()

### Code Example
```cpp
// Single instance check
CreateMutexA(0, FALSE, "capsicain_release");
if (GetLastError() == ERROR_ALREADY_EXISTS)
    return false;  // Another instance running
```

### Related Files
- capsicain/capsicain.cpp (initConsoleWindow)
- capsicain/traybar.cpp
- capsicain/utils.cpp
