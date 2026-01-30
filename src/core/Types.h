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
#include <array>
#include "platform/constants.h"
#include "domain/ComboMatcher.h"

namespace capsicain {

// Modifier bitmask type
using ModifierMask = uint32_t;
using MOD = uint32_t;  // Legacy alias

// Device bitmask type
using DeviceMask = uint32_t;
using DEV = uint32_t;  // Legacy alias (defined in constants.h too)

// Virtual key code type
using VKey = int;

// Note: Constants like MAX_VCODES, REWIRE_COLS, etc. are defined in platform/constants.h as #defines
// to maintain compatibility with legacy code.

/**
 * @brief Key event (vcode + direction)
 */
struct VKeyEvent {
    int vcode = 0;
    bool isDownstroke = true;
};

/**
 * @brief Device information
 */
struct Device {
    std::string id;
    bool keyboard = false;
    bool apple = false;
};

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
 * @brief Modifier combo configuration
 */
struct ModifierCombo {
    int vkey = 0;  // SC_NOP
    unsigned char deadkey = 0;
    MOD modAnd = 0;
    MOD modOr = 0;
    MOD modNot = 0;
    MOD modTap = 0;
    MOD modTapAnd = 0;
    DEV devAnd = 0;
    DEV devNot = 0;
    std::vector<VKeyEvent> keyEventSequence;
};

/**
 * @brief Mapping data (replaces allMaps global)
 * Contains all keyboard remapping configuration
 */
struct MappingData {
    // Rewire map: [scancode][REWIRE_OUT/TAP/DOUBLETAP/TAPHOLD] -> output scancode
    std::array<std::array<int, REWIRE_COLS>, REWIRE_ROWS> rewiremap{};

    // Modifier combos by type
    std::map<std::string, std::vector<ModifierCombo>> modCombos;

    // Cached converted combos for performance
    std::map<std::string, std::vector<domain::ComboRule>> convertedCombos;

    // Alpha mapping: [scancode] -> output scancode
    std::array<int, MAX_VCODES> alphamap{};

    // Executables mapped to keys
    std::map<int, Executable> executables;

    // Device information
    std::map<uint8_t, Device> devices;
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
