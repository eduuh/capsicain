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
#include <set>
#include "platform/constants.h"

namespace capsicain {

// Modifier bitmask type
using ModifierMask = uint32_t;

// Device bitmask type  
using DeviceMask = uint32_t;

// Virtual key code type
using VKey = int;

// Note: Constants like MAX_VCODES, REWIRE_COLS, etc. are defined in platform/constants.h as #defines
// to maintain compatibility with legacy code.

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
    int delayForKeySequenceMS = DEFAULT_DELAY_FOR_KEY_SEQUENCE_MS;
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
    int activeConfigOnStartup = DEFAULT_ACTIVE_CONFIG;
    bool startMinimized = false;
    bool startInTraybar = false;
    bool startAHK = false;
    int capsicainOnOffKey = -1;
    bool protectConsole = true;
    bool translateMessyKeys = true;
    std::set<int> disableEscKey;
    std::set<int> forwardEscKey;
};

} // namespace capsicain
