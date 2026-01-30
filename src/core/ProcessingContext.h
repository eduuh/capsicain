#pragma once
/**
 * @file ProcessingContext.h
 * @brief Context object for key processing pipeline
 * 
 * Replaces the global loopState, modifierState, and related globals
 * with an explicit context that flows through the processing pipeline.
 */

#include "KeyEvent.h"
#include "../modifiers.h"
#include <chrono>

namespace capsicain {

/**
 * @brief Immutable input for the processing pipeline
 * 
 * Contains all the information about the current key event
 * and the state needed to process it.
 */
struct ProcessingInput {
    KeyEvent event;                     ///< The key event being processed
    uint8_t scancode = 0;               ///< Original hardware scancode
    DeviceInfo device;                  ///< Source device information
    
    // Previous events for tap detection
    KeyEvent previousEvent1;
    KeyEvent previousEvent2;
    
    std::chrono::steady_clock::time_point timestamp;
};

/**
 * @brief Mutable state during processing
 * 
 * This replaces the loopState global. It tracks what's happening
 * to a key as it moves through the processing stages.
 */
struct ProcessingState {
    int vcode = -1;                     ///< Current virtual key code (may change during processing)
    bool isDownstroke = false;
    bool isModifier = false;
    
    // Tap detection results
    bool tapped = false;
    bool tappedSlow = false;            ///< Autorepeat happened before release
    bool tapHoldMake = false;           ///< Tap-and-hold action triggered
    bool repeat = false;                ///< Key is repeating
    
    // Processing result
    KeySequence resultSequence;         ///< Output key events
    
    void reset() {
        vcode = -1;
        isDownstroke = false;
        isModifier = false;
        tapped = false;
        tappedSlow = false;
        tapHoldMake = false;
        repeat = false;
        resultSequence.clear();
    }
};

/**
 * @brief Modifier key tracking state
 * 
 * Tracks which modifier keys are currently pressed and
 * their tap state.
 */
struct ModifierState {
    uint8_t activeDeadkey = 0;          ///< Current deadkey (not really a modifier)
    MOD modifierDown = 0;               ///< Bitmask of currently held modifiers
    MOD modifierTapped = 0;             ///< Bitmask of tapped modifiers
    MOD modifierForceDown = 0;          ///< Modifiers forced down by combos
    
    KeySequence modsTempAltered;        ///< Temporarily altered modifiers
    int tapAndHoldKey = -1;             ///< Key held for tap-and-hold
    
    void reset() {
        activeDeadkey = 0;
        modifierDown = 0;
        modifierTapped = 0;
        modifierForceDown = 0;
        modsTempAltered.clear();
        tapAndHoldKey = -1;
    }
    
    bool isShiftDown() const {
        return (modifierDown & BITMASK_LSHIFT) || (modifierDown & BITMASK_RSHIFT);
    }
    
    bool isCtrlDown() const {
        return (modifierDown & BITMASK_LCTRL) || (modifierDown & BITMASK_RCTRL);
    }
    
    bool isAltDown() const {
        return (modifierDown & BITMASK_LALT) || (modifierDown & BITMASK_RALT);
    }
    
    bool isWinDown() const {
        return (modifierDown & BITMASK_LWIN) || (modifierDown & BITMASK_RWIN);
    }
};

/**
 * @brief Complete processing context
 * 
 * This is the main context object that flows through the processing pipeline.
 * It contains all state needed for processing and replaces multiple globals.
 */
class ProcessingContext {
public:
    ProcessingInput input;
    ProcessingState state;
    ModifierState modifiers;
    
    // Configuration reference (set by service layer)
    int activeConfig = 0;
    std::string activeConfigName;
    
    // Processing flags
    bool shouldForward = false;         ///< Just forward without processing
    bool shouldDrop = false;            ///< Drop the key entirely
    
    void reset() {
        state.reset();
        shouldForward = false;
        shouldDrop = false;
    }
    
    /**
     * @brief Get the final output sequence
     */
    const KeySequence& getOutput() const {
        return state.resultSequence;
    }
    
    /**
     * @brief Add a key event to the output
     */
    void addOutput(const KeyEvent& event) {
        state.resultSequence.push_back(event);
    }
    
    /**
     * @brief Add a key event to the output (convenience overload)
     */
    void addOutput(int vcode, bool isDown) {
        state.resultSequence.emplace_back(vcode, isDown);
    }
};

} // namespace capsicain
