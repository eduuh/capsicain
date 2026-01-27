## Learning Task: File I/O and INI Parsing

### Objective
Learn file reading, string parsing, and configuration file handling.

### Location
- **File:** capsicain/configUtils.cpp

### What You'll Learn
1. File stream operations (ifstream)
2. Line-by-line file reading
3. String tokenization and parsing
4. Section-based INI file structure
5. Input validation and error handling

### Key Functions to Study
- `readSanitizeIniFile()` - Main file reading
- `normalizeLine()` - Text preprocessing
- `getSectionFromIni()` - Extract config sections
- `getTaggedLinesFromIni()` - Find tagged lines
- `parseKeywordRewire()` - Parse specific syntax

### Parsing Flow
```
capsicain.ini
     │
     ▼
readSanitizeIniFile() ──► sanitizedIniContent vector
     │
     ▼
getSectionFromIni("[config_1]") ──► section lines
     │
     ▼
parseIniRewires(), parseIniCombos(), etc.
```

### Code Example
```cpp
bool readSanitizeIniFile(std::vector<string> &iniLines)
{
    ifstream f("capsicain.ini");
    if (!f.is_open())
        return false;

    string line;
    while (getline(f, line)) 
    {
        normalizeLine(line);   // Clean up the line
        if (line == "")
            continue;
        iniLines.push_back(line);
    }
    return !f.bad();
}
```

### Tasks
- [ ] Trace readSanitizeIniFile() step by step
- [ ] Understand normalizeLine() transformations
- [ ] Study how sections are extracted with getSectionFromIni()
- [ ] Understand the INCLUDE directive processing
- [ ] Follow parseKeywordRewire() parsing logic

### Related Files
- capsicain/configUtils.cpp
- capsicain/configUtils.h
- capsicain/capsicain.ini (example config)
