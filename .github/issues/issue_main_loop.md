## Learning Task: Understand the Main Event Loop

### Objective
Trace the core keyboard event processing loop from start to finish.

### Location
- **File:** capsicain/capsicain.cpp (main function, ~line 365)

### What You'll Learn
1. Main function structure in C++
2. Initialization and cleanup patterns
3. Event-driven programming
4. State machine concepts
5. Real-time input processing

### Event Loop Overview
```
┌─────────────────────────────────────────────────────────┐
│                      main()                              │
├─────────────────────────────────────────────────────────┤
│  1. initConsoleWindow()                                  │
│  2. interception_create_context()                        │
│  3. readSanitizeIniFile()                               │
│  4. parseIniGlobals()                                   │
│  5. switchConfig()                                       │
│  6. interception_set_filter()                           │
│                                                          │
│  ┌─────── CORE LOOP ──────────────────────────────────┐ │
│  │  while (!exit)                                      │ │
│  │  {                                                  │ │
│  │    interception_receive() ─► Get key event          │ │
│  │    processOnOffKey()                                │ │
│  │    processMessyKeys()                               │ │
│  │    detectTapping()                                  │ │
│  │    processRewireScancodeToVirtualcode()             │ │
│  │    processModifierState()                           │ │
│  │    processCombos()                                  │ │
│  │    processMapAlphaKeys()                            │ │
│  │    sendResultingKeyOrSequence()                     │ │
│  │  }                                                  │ │
│  └─────────────────────────────────────────────────────┘ │
│                                                          │
│  7. interception_destroy_context()                       │
└─────────────────────────────────────────────────────────┘
```

### Tasks
- [ ] Find main() and trace initialization order
- [ ] Understand the while loop structure
- [ ] Trace a single keypress through all processing functions
- [ ] Understand how ESC commands are handled
- [ ] Identify the cleanup code

### Key Variables in the Loop
- `interceptionState.currentIKstroke` - Current key event
- `loopState` - Processing state for current key
- `modifierState` - Current modifier key states
- `globalState` - Application-wide state

### Related Files
- capsicain/capsicain.cpp
- capsicain/interception.h
