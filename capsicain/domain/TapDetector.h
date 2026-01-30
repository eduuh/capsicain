#pragma once
/**
 * @file TapDetector.h
 * @brief Detects tap, slow-tap, tap-hold patterns
 * 
 * This is a pure logic component with no dependencies on global state.
 * It can be unit tested in isolation.
 */

#include "../core/KeyEvent.h"

namespace capsicain {

/**
 * @brief Result of tap detection analysis
 */
struct TapResult {
    bool tapped = false;        ///< Quick tap (down immediately followed by up)
    bool tappedSlow = false;    ///< Slow tap (key repeated before release)
    bool tapHoldMake = false;   ///< Tap-and-hold pattern (down-up-down)
    bool repeat = false;        ///< Key is auto-repeating
    
    void reset() {
        tapped = false;
        tappedSlow = false;
        tapHoldMake = false;
        repeat = false;
    }
};

/**
 * @brief Detects tap patterns from key event sequences
 * 
 * Analyzes the current key event and the two previous events
 * to detect various tap patterns:
 * 
 * - **Tap**: Key pressed and released without any intermediate keys
 * - **Slow tap**: Key pressed, auto-repeated, then released
 * - **Tap-hold**: Key tapped then immediately pressed again (down-up-down)
 * - **Repeat**: Key is auto-repeating (down-down same key)
 * 
 * Usage:
 * ```cpp
 * TapDetector detector;
 * TapResult result = detector.detect(current, prev1, prev2);
 * if (result.tapped) {
 *     // Handle tap
 * }
 * ```
 */
class TapDetector {
public:
    /**
     * @brief Analyze key events to detect tap patterns
     * 
     * @param current Current key event being processed
     * @param prev1 Previous key event (most recent)
     * @param prev2 Key event before prev1
     * @param currentState Raw key state from hardware
     * @param prev1State Previous raw key state
     * @param prev2State Key state before prev1
     * @return TapResult with detected patterns
     */
    TapResult detect(
        const KeyEvent& current,
        const KeyEvent& prev1,
        const KeyEvent& prev2,
        uint8_t currentState,
        uint8_t prev1State,
        uint8_t prev2State
    ) const {
        TapResult result;
        
        // Tapped key = key up, same as previous key, previous was key down
        result.tapped = 
            !current.isDownstroke
            && (current.vcode == prev1.vcode)
            && ((prev1State & 1) == 0);  // prev1 was down
        
        // Slow tap = tapped but there was an auto-repeat before release
        // Pattern: down -> down (repeat) -> up
        result.tappedSlow = 
            result.tapped
            && (prev2.vcode == current.vcode)
            && ((prev2State & 1) == 0);  // prev2 was also down
        
        // Slow tap overrides regular tap
        if (result.tappedSlow) {
            result.tapped = false;
        }
        
        // Tap-hold make = down-up-down sequence for same key
        // Pattern: prev2=down, prev1=up, current=down (same key for all three)
        if (prev1.vcode == current.vcode
            && prev2.vcode == current.vcode
            && ((currentState & 1) == 0)   // current is down
            && ((prev1State & 1) == 1)     // prev1 is up
            && ((prev2State & 1) == 0))    // prev2 is down
        {
            result.tapHoldMake = true;
        }
        
        // Repeat = two consecutive downs for same key
        if (((currentState & 1) == 0)      // current is down
            && ((prev1State & 1) == 0)      // prev1 is down
            && (prev1.vcode == current.vcode))
        {
            result.repeat = true;
        }
        
        return result;
    }
    
    /**
     * @brief Simplified detection using KeyEvent only
     * 
     * Use this when you don't have access to raw hardware state.
     * Derives state from KeyEvent.isDownstroke.
     */
    TapResult detect(
        const KeyEvent& current,
        const KeyEvent& prev1,
        const KeyEvent& prev2
    ) const {
        // Convert isDownstroke to state (0 = down, 1 = up)
        uint8_t currentState = current.isDownstroke ? 0 : 1;
        uint8_t prev1State = prev1.isDownstroke ? 0 : 1;
        uint8_t prev2State = prev2.isDownstroke ? 0 : 1;
        
        return detect(current, prev1, prev2, currentState, prev1State, prev2State);
    }
};

} // namespace capsicain
