#pragma once
/**
 * @file Types.h
 * @brief Common type definitions for capsicain
 * 
 * Centralizes type aliases and constants used throughout the codebase.
 */

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace capsicain {

// Modifier bitmask type
using ModifierMask = uint32_t;

// Device bitmask type  
using DeviceMask = uint32_t;

// Virtual key code type
using VKey = int;

// Constants
constexpr int MAX_VCODES = 0x140;
constexpr int MAX_MACRO_LENGTH = 200;
constexpr int MAX_NUM_MACROS = 21;
constexpr int DISABLED_CONFIG = 0;
constexpr int DEFAULT_CONFIG = 1;
constexpr int DEFAULT_KEY_DELAY_MS = 5;

// Rewire map dimensions
constexpr int REWIRE_COLS = 4;
constexpr int REWIRE_ROWS = MAX_VCODES;
constexpr int REWIRE_OUT = 0;
constexpr int REWIRE_TAP = 1;
constexpr int REWIRE_DOUBLETAP = 2;
constexpr int REWIRE_TAPHOLD = 3;

/**
 * @brief Executable configuration for launching programs
 */
struct Executable {
    std::string verb;
    std::string path;
    std::string args;
    std::string dir;
    int mode = 0;
    void* proc = nullptr;
    uint32_t pid = 0;
    void* hwnd = nullptr;
};

/**
 * @brief Configuration options that can be toggled at runtime
 */
struct RuntimeOptions {
    bool debug = false;
    int delayForKeySequenceMS = DEFAULT_KEY_DELAY_MS;
    bool flipZY = false;
    bool flipAltWinOnAppleKeyboards = false;
    bool lControlLWinBlocksAlphaMapping = false;
    bool processOnlyFirstKeyboard = false;
    bool holdRepeatsAllKeys = false;
    bool disableAHKDelay = false;
    bool enableMouse = false;
    std::string defaultFunction = "key(%s, m)";
};

/**
 * @brief Global settings loaded from INI (not changeable at runtime)
 */
struct GlobalSettings {
    std::string iniVersion = "unnamed version";
    int activeConfigOnStartup = DEFAULT_CONFIG;
    bool startMinimized = false;
    bool startInTraybar = false;
    bool startAHK = false;
    int capsicainOnOffKey = -1;
    bool protectConsole = true;
    bool translateMessyKeys = true;
};

} // namespace capsicain
