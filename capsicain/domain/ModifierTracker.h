#pragma once
/**
 * @file ModifierTracker.h
 * @brief Tracks modifier key states
 * 
 * This is a pure logic component that tracks which modifier keys
 * are currently pressed and their tap state.
 */

#include "../core/KeyEvent.h"
#include "../modifiers.h"

namespace capsicain {

/**
 * @brief Tracks modifier key state
 * 
 * Maintains a bitmask of which modifier keys are currently held down,
 * which have been tapped, and which are forced down by combos.
 * 
 * Usage:
 * ```cpp
 * ModifierTracker tracker;
 * tracker.update(vcode, isDown, isTapped);
 * if (tracker.isShiftDown()) { ... }
 * ```
 */
class ModifierTracker {
public:
    /**
     * @brief Update modifier state based on a key event
     * 
     * @param vcode Virtual key code
     * @param isDownstroke true if key is being pressed
     * @param isTapped true if key was tapped
     */
    void update(int vcode, bool isDownstroke, bool isTapped) {
        MOD modBitmask = getModifierBitmaskForVcode(vcode);
        
        if (modBitmask == 0) {
            return; // Not a modifier key
        }
        
        // Update down state
        if (isDownstroke) {
            modifierDown_ |= modBitmask;
        } else {
            modifierDown_ &= ~modBitmask;
        }
        
        // Tapped modifier sets tapped bitmask
        // You can combine mod-taps (like tap-Ctrl then tap-Alt)
        if (isTapped) {
            modifierTapped_ |= modBitmask;
        }
        
        // Apply forced modifiers
        modifierDown_ |= modifierForceDown_;
    }
    
    /**
     * @brief Clear tapped state for a specific modifier
     */
    void clearTapped(int vcode) {
        MOD modBitmask = getModifierBitmaskForVcode(vcode);
        if (modBitmask != 0) {
            modifierTapped_ &= ~modBitmask;
        }
    }
    
    /**
     * @brief Clear all tapped states
     */
    void clearAllTapped() {
        modifierTapped_ = 0;
    }
    
    /**
     * @brief Clear down state for a specific modifier
     */
    void clearDown(int vcode) {
        MOD modBitmask = getModifierBitmaskForVcode(vcode);
        if (modBitmask != 0) {
            modifierDown_ &= ~modBitmask;
        }
    }
    
    /**
     * @brief Force a modifier to be considered "down"
     */
    void forceDown(int vcode) {
        MOD modBitmask = getModifierBitmaskForVcode(vcode);
        if (modBitmask != 0) {
            modifierForceDown_ |= modBitmask;
            modifierDown_ |= modBitmask;
        }
    }
    
    /**
     * @brief Clear a forced modifier
     */
    void clearForced(int vcode) {
        MOD modBitmask = getModifierBitmaskForVcode(vcode);
        if (modBitmask != 0) {
            modifierForceDown_ &= ~modBitmask;
        }
    }
    
    /**
     * @brief Reset all modifier state
     */
    void reset() {
        modifierDown_ = 0;
        modifierTapped_ = 0;
        modifierForceDown_ = 0;
        activeDeadkey_ = 0;
        tapAndHoldKey_ = -1;
    }
    
    // Convenience query methods
    bool isShiftDown() const {
        return (modifierDown_ & BITMASK_LSHIFT) || (modifierDown_ & BITMASK_RSHIFT);
    }
    
    bool isLShiftDown() const {
        return modifierDown_ & BITMASK_LSHIFT;
    }
    
    bool isRShiftDown() const {
        return modifierDown_ & BITMASK_RSHIFT;
    }
    
    bool isCtrlDown() const {
        return (modifierDown_ & BITMASK_LCTRL) || (modifierDown_ & BITMASK_RCTRL);
    }
    
    bool isLCtrlDown() const {
        return modifierDown_ & BITMASK_LCTRL;
    }
    
    bool isRCtrlDown() const {
        return modifierDown_ & BITMASK_RCTRL;
    }
    
    bool isAltDown() const {
        return (modifierDown_ & BITMASK_LALT) || (modifierDown_ & BITMASK_RALT);
    }
    
    bool isLAltDown() const {
        return modifierDown_ & BITMASK_LALT;
    }
    
    bool isRAltDown() const {
        return modifierDown_ & BITMASK_RALT;
    }
    
    bool isWinDown() const {
        return (modifierDown_ & BITMASK_LWIN) || (modifierDown_ & BITMASK_RWIN);
    }
    
    bool isLWinDown() const {
        return modifierDown_ & BITMASK_LWIN;
    }
    
    bool isRWinDown() const {
        return modifierDown_ & BITMASK_RWIN;
    }
    
    /**
     * @brief Check if a specific modifier is down
     */
    bool isModifierDown(int vcode) const {
        MOD modBitmask = getModifierBitmaskForVcode(vcode);
        return modBitmask != 0 && (modifierDown_ & modBitmask);
    }
    
    /**
     * @brief Check if a specific modifier was tapped
     */
    bool isModifierTapped(int vcode) const {
        MOD modBitmask = getModifierBitmaskForVcode(vcode);
        return modBitmask != 0 && (modifierTapped_ & modBitmask);
    }
    
    /**
     * @brief Check if modifiers match a pattern
     * 
     * @param andMask All these modifiers must be down
     * @param orMask At least one of these must be down (0 = don't care)
     * @param notMask None of these may be down
     * @return true if pattern matches
     */
    bool matchesPattern(MOD andMask, MOD orMask, MOD notMask) const {
        // All 'and' modifiers must be down
        if ((modifierDown_ & andMask) != andMask) {
            return false;
        }
        
        // At least one 'or' modifier must be down (if orMask != 0)
        if (orMask != 0 && (modifierDown_ & orMask) == 0) {
            return false;
        }
        
        // None of 'not' modifiers may be down
        if ((modifierDown_ & notMask) != 0) {
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief Check if tapped modifiers match a pattern
     */
    bool matchesTappedPattern(MOD andMask) const {
        return (modifierTapped_ & andMask) == andMask;
    }
    
    // Accessors for raw bitmasks (for compatibility)
    MOD getDownMask() const { return modifierDown_; }
    MOD getTappedMask() const { return modifierTapped_; }
    MOD getForcedMask() const { return modifierForceDown_; }
    
    // Deadkey support
    void setDeadkey(uint8_t dk) { activeDeadkey_ = dk; }
    uint8_t getDeadkey() const { return activeDeadkey_; }
    void clearDeadkey() { activeDeadkey_ = 0; }
    
    // Tap-and-hold key tracking
    void setTapHoldKey(int key) { tapAndHoldKey_ = key; }
    int getTapHoldKey() const { return tapAndHoldKey_; }
    void clearTapHoldKey() { tapAndHoldKey_ = -1; }
    bool hasTapHoldKey() const { return tapAndHoldKey_ >= 0; }
    
private:
    MOD modifierDown_ = 0;          ///< Bitmask of currently held modifiers
    MOD modifierTapped_ = 0;        ///< Bitmask of tapped modifiers
    MOD modifierForceDown_ = 0;     ///< Modifiers forced down by combos
    uint8_t activeDeadkey_ = 0;     ///< Active deadkey (not really a modifier)
    int tapAndHoldKey_ = -1;        ///< Key held for tap-and-hold (-1 = none)
};

} // namespace capsicain
