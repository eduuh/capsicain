#pragma once
/**
 * TapDance.h
 * 
 * QMK-inspired Tap Dance implementation for Interception.
 * Allows a single key to perform different actions based on:
 * - Number of consecutive taps (1, 2, 3...)
 * - Whether the key is held after tapping
 * 
 * Example configurations:
 * - Single tap: Escape
 * - Double tap: Caps Lock
 * - Single hold: Control
 * - Double tap + hold: Toggle layer
 */

#include <cstdint>
#include <vector>
#include <functional>
#include <chrono>

namespace capsicain {
namespace domain {

/**
 * Tap dance action types
 */
enum class TapDanceAction : uint8_t {
    NONE = 0,
    SINGLE_TAP,         // Tapped once
    SINGLE_HOLD,        // Held after first press
    DOUBLE_TAP,         // Tapped twice quickly
    DOUBLE_HOLD,        // Held after second tap
    DOUBLE_SINGLE_TAP,  // Fast typing (e.g., "pp" in "pepper")
    TRIPLE_TAP,         // Tapped three times
    TRIPLE_HOLD,        // Held after third tap
};

/**
 * State tracking for a tap dance key
 */
struct TapDanceState {
    uint16_t triggerKey = 0;            // The key that triggers this tap dance
    uint8_t tapCount = 0;               // Number of taps so far
    bool isPressed = false;             // Currently held down
    bool isInterrupted = false;         // Another key was pressed during the dance
    bool isFinished = false;            // Dance has resolved
    TapDanceAction resolvedAction = TapDanceAction::NONE;
    
    // Timing
    std::chrono::steady_clock::time_point lastTapTime;
    
    void reset() {
        tapCount = 0;
        isPressed = false;
        isInterrupted = false;
        isFinished = false;
        resolvedAction = TapDanceAction::NONE;
    }
};

/**
 * Configuration for a single tap dance key
 */
struct TapDanceConfig {
    uint16_t triggerKey = 0;            // Key that activates this tap dance
    uint16_t tappingTermMs = 200;       // Time window for taps (default 200ms like QMK)
    
    // Actions for each state (0 = pass through, else keycode to send)
    uint16_t singleTap = 0;
    uint16_t singleHold = 0;
    uint16_t doubleTap = 0;
    uint16_t doubleHold = 0;
    uint16_t tripleTap = 0;
    uint16_t tripleHold = 0;
    
    // For advanced use: custom callbacks
    std::function<void(TapDanceAction)> onFinish = nullptr;
    std::function<void()> onReset = nullptr;
};

/**
 * Result of processing a key through tap dance
 */
struct TapDanceResult {
    bool consumed = false;              // Whether the input was consumed
    bool waitingForMore = false;        // Still waiting for more taps or timeout
    TapDanceAction action = TapDanceAction::NONE;
    uint16_t outputKey = 0;             // Key to send (if any)
    bool sendKeyDown = false;
    bool sendKeyUp = false;
};

/**
 * TapDanceEngine - Manages multiple tap dance keys
 * 
 * Pure logic, no I/O dependencies. Uses injected time for testability.
 */
class TapDanceEngine {
public:
    using TimePoint = std::chrono::steady_clock::time_point;
    
    /**
     * Add a tap dance configuration
     */
    void addTapDance(const TapDanceConfig& config) {
        m_configs.push_back(config);
        m_states.push_back(TapDanceState{});
        m_states.back().triggerKey = config.triggerKey;
    }
    
    /**
     * Process a key event through the tap dance engine
     * 
     * @param keycode The key being pressed/released
     * @param isDown Key direction
     * @param now Current time (injected for testability)
     * @return Processing result
     */
    TapDanceResult process(uint16_t keycode, bool isDown, TimePoint now) {
        TapDanceResult result;
        
        // Find if this key has a tap dance
        for (size_t i = 0; i < m_configs.size(); ++i) {
            if (m_configs[i].triggerKey == keycode) {
                result = processTapDanceKey(i, isDown, now);
                return result;
            }
        }
        
        // Not a tap dance key - check if it interrupts any active dances
        for (size_t i = 0; i < m_states.size(); ++i) {
            if (m_states[i].tapCount > 0 && !m_states[i].isFinished) {
                m_states[i].isInterrupted = true;
                resolveTapDance(i, now);
            }
        }
        
        return result;  // Not consumed
    }
    
    /**
     * Check for timed-out tap dances (call periodically)
     */
    std::vector<TapDanceResult> checkTimeouts(TimePoint now) {
        std::vector<TapDanceResult> results;
        
        for (size_t i = 0; i < m_states.size(); ++i) {
            if (m_states[i].tapCount > 0 && 
                !m_states[i].isFinished && 
                !m_states[i].isPressed) {
                
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - m_states[i].lastTapTime
                ).count();
                
                if (elapsed >= m_configs[i].tappingTermMs) {
                    resolveTapDance(i, now);
                    results.push_back(createResult(i));
                }
            }
        }
        
        return results;
    }
    
private:
    TapDanceResult processTapDanceKey(size_t index, bool isDown, TimePoint now) {
        TapDanceState& state = m_states[index];
        const TapDanceConfig& config = m_configs[index];
        TapDanceResult result;
        result.consumed = true;
        
        if (isDown) {
            // Key pressed
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - state.lastTapTime
            ).count();
            
            // New tap or continuation?
            if (state.tapCount == 0 || elapsed < config.tappingTermMs) {
                state.tapCount++;
                state.isPressed = true;
                state.lastTapTime = now;
                result.waitingForMore = true;
            } else {
                // Timeout passed, this is a new dance
                state.reset();
                state.tapCount = 1;
                state.isPressed = true;
                state.lastTapTime = now;
                result.waitingForMore = true;
            }
        } else {
            // Key released
            state.isPressed = false;
            
            // If interrupted or timed out, resolve now
            if (state.isInterrupted) {
                resolveTapDance(index, now);
                result = createResult(index);
            } else {
                // Wait for possible more taps
                result.waitingForMore = true;
            }
        }
        
        return result;
    }
    
    void resolveTapDance(size_t index, TimePoint now) {
        TapDanceState& state = m_states[index];
        
        if (state.isFinished) return;
        state.isFinished = true;
        
        // Determine action based on count and hold state
        if (state.tapCount == 1) {
            if (state.isInterrupted || !state.isPressed) {
                state.resolvedAction = TapDanceAction::SINGLE_TAP;
            } else {
                state.resolvedAction = TapDanceAction::SINGLE_HOLD;
            }
        } else if (state.tapCount == 2) {
            if (state.isInterrupted) {
                state.resolvedAction = TapDanceAction::DOUBLE_SINGLE_TAP;
            } else if (state.isPressed) {
                state.resolvedAction = TapDanceAction::DOUBLE_HOLD;
            } else {
                state.resolvedAction = TapDanceAction::DOUBLE_TAP;
            }
        } else if (state.tapCount >= 3) {
            if (!state.isPressed) {
                state.resolvedAction = TapDanceAction::TRIPLE_TAP;
            } else {
                state.resolvedAction = TapDanceAction::TRIPLE_HOLD;
            }
        }
    }
    
    TapDanceResult createResult(size_t index) {
        TapDanceResult result;
        result.consumed = true;
        result.action = m_states[index].resolvedAction;
        
        const TapDanceConfig& config = m_configs[index];
        
        switch (result.action) {
            case TapDanceAction::SINGLE_TAP:
                result.outputKey = config.singleTap;
                result.sendKeyDown = true;
                result.sendKeyUp = true;
                break;
            case TapDanceAction::SINGLE_HOLD:
                result.outputKey = config.singleHold;
                result.sendKeyDown = true;  // Will send up on release
                break;
            case TapDanceAction::DOUBLE_TAP:
                result.outputKey = config.doubleTap;
                result.sendKeyDown = true;
                result.sendKeyUp = true;
                break;
            case TapDanceAction::DOUBLE_HOLD:
                result.outputKey = config.doubleHold;
                result.sendKeyDown = true;
                break;
            case TapDanceAction::TRIPLE_TAP:
                result.outputKey = config.tripleTap;
                result.sendKeyDown = true;
                result.sendKeyUp = true;
                break;
            default:
                break;
        }
        
        // Call custom handler if defined
        if (config.onFinish) {
            config.onFinish(result.action);
        }
        
        return result;
    }
    
    std::vector<TapDanceConfig> m_configs;
    std::vector<TapDanceState> m_states;
};

} // namespace domain
} // namespace capsicain
