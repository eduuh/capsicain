#pragma once
/**
 * @file KeyEvent.h
 * @brief Core key event data structures
 * 
 * This file contains the fundamental data types for representing
 * keyboard events throughout the processing pipeline.
 */

#include <cstdint>
#include <vector>

namespace capsicain {

/**
 * @brief Represents a single key event (press or release)
 * 
 * This is the core data type that flows through the processing pipeline.
 * It replaces the old VKeyEvent struct with a more descriptive interface.
 */
struct KeyEvent {
    int vcode = 0;              ///< Virtual key code (can be scancode or extended code)
    bool isDownstroke = true;   ///< true = key pressed, false = key released
    
    KeyEvent() = default;
    KeyEvent(int code, bool down) : vcode(code), isDownstroke(down) {}
    
    bool operator==(const KeyEvent& other) const {
        return vcode == other.vcode && isDownstroke == other.isDownstroke;
    }
    
    bool operator!=(const KeyEvent& other) const {
        return !(*this == other);
    }
};

/**
 * @brief A sequence of key events (used for macros, combos, etc.)
 */
using KeySequence = std::vector<KeyEvent>;

/**
 * @brief Keyboard state flags used during key processing
 */
enum class KeyState : uint8_t {
    DOWN = 0,
    UP = 1,
    E0_DOWN = 2,
    E0_UP = 3,
    E1_DOWN = 4,
    E1_UP = 5
};

/**
 * @brief Device type classification
 */
enum class DeviceType {
    Unknown,
    Keyboard,
    AppleKeyboard,
    Mouse
};

/**
 * @brief Information about an input device
 */
struct DeviceInfo {
    std::string id;
    DeviceType type = DeviceType::Unknown;
    bool isApple = false;
    
    bool isKeyboard() const { 
        return type == DeviceType::Keyboard || type == DeviceType::AppleKeyboard; 
    }
};

} // namespace capsicain
