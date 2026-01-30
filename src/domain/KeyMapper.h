#pragma once
/**
 * KeyMapper.h
 * 
 * Pure domain logic for key remapping operations.
 * Extracted from processMapAlphaKeys() and processRewireScancodeToVirtualcode()
 * for testability.
 * 
 * Provides:
 * - Alpha key mapping (simple key-to-key remapping)
 * - Rewire mapping (scancode to virtualcode with tap/taphold support)
 * - Z/Y flip for international keyboard layouts
 */

#include <vector>
#include <cstdint>
#include <array>

namespace capsicain {
namespace domain {

// Forward-compatible type aliases
using VKeyCode = uint16_t;
using ScanCode = uint16_t;
using MOD = uint32_t;

// Constants
constexpr VKeyCode SC_NOP = 0xFF;
constexpr int REWIRE_UNDEFINED = -1;

// MAX_VCODES: Use platform definition if available, otherwise fallback
#ifndef MAX_VCODES
constexpr size_t MAX_VCODES = 256;
#endif

// Note: REWIRE_OUT, REWIRE_TAP, REWIRE_TAPHOLD, REWIRE_COLS
// are defined in platform/constants.h and will be available when this header is used

/**
 * Key event for output sequences
 */
struct KeyMapEvent {
    VKeyCode keyCode = SC_NOP;
    bool isDown = true;

    constexpr KeyMapEvent() noexcept = default;
    constexpr KeyMapEvent(VKeyCode k, bool d) noexcept : keyCode(k), isDown(d) {}

    constexpr bool operator==(const KeyMapEvent& other) const noexcept {
        return keyCode == other.keyCode && isDown == other.isDown;
    }
};

/**
 * Result of alpha key mapping
 */
struct AlphaMapResult {
    VKeyCode mappedKey = SC_NOP;    // The resulting key after mapping
    bool wasRemapped = false;        // Whether any mapping occurred
};

/**
 * Options for alpha mapping
 */
struct AlphaMapOptions {
    bool flipZY = false;                        // Swap Z and Y keys
    bool ctrlWinBlocksAlphaMapping = false;     // Skip mapping when Ctrl or Win is held
};

/**
 * Pure function to apply alpha key mapping.
 *
 * @param inputKey The key to map
 * @param alphaMap Array mapping vcodes to their remapped values (can be nullptr)
 * @param options Mapping options
 * @param isModifierKey Whether the input key is a modifier
 * @param isCtrlDown Whether Ctrl is currently held
 * @param isWinDown Whether Win is currently held
 * @return The mapping result
 */
inline AlphaMapResult applyAlphaMapping(
    VKeyCode inputKey,
    const int* alphaMap,  // Array of MAX_VCODES elements (nullable)
    const AlphaMapOptions& options,
    bool isModifierKey,
    bool isCtrlDown,
    bool isWinDown
) noexcept {
    AlphaMapResult result;
    result.mappedKey = inputKey;
    result.wasRemapped = false;
    
    // Skip mapping for modifier keys
    if (isModifierKey) {
        return result;
    }
    
    // Skip mapping when Ctrl+Win blocks it
    if (options.ctrlWinBlocksAlphaMapping && (isCtrlDown || isWinDown)) {
        return result;
    }
    
    // Apply alpha map
    if (inputKey < MAX_VCODES && alphaMap != nullptr) {
        int mapped = alphaMap[inputKey];
        if (mapped >= 0 && mapped != static_cast<int>(inputKey)) {
            result.mappedKey = static_cast<VKeyCode>(mapped);
            result.wasRemapped = true;
        } else if (mapped >= 0) {
            result.mappedKey = static_cast<VKeyCode>(mapped);
        }
    }
    
    // Apply Z/Y flip
    if (options.flipZY) {
        constexpr VKeyCode SC_Y = 0x15;  // Y scancode
        constexpr VKeyCode SC_Z = 0x2C;  // Z scancode
        
        if (result.mappedKey == SC_Y) {
            result.mappedKey = SC_Z;
            result.wasRemapped = true;
        } else if (result.mappedKey == SC_Z) {
            result.mappedKey = SC_Y;
            result.wasRemapped = true;
        }
    }
    
    return result;
}


/**
 * Rewire map entry for a single key
 */
struct RewireEntry {
    int outKey = REWIRE_UNDEFINED;      // Primary output key (-1 = no mapping)
    int tapKey = REWIRE_UNDEFINED;      // Key to send on tap (-1 = no tap action)
    int tapHoldKey = REWIRE_UNDEFINED;  // Key to send on tap-hold (-1 = no taphold action)

    constexpr bool hasOutput() const noexcept { return outKey >= 0; }
    constexpr bool hasTap() const noexcept { return tapKey >= 0; }
    constexpr bool hasTapHold() const noexcept { return tapHoldKey >= 0; }
};

/**
 * Context for rewire processing
 */
struct RewireContext {
    ScanCode scancode = 0;              // Original hardware scancode
    VKeyCode vcode = 0;                 // Current virtual keycode
    bool isDownstroke = true;           // Key direction
    bool isTapped = false;              // Whether key was tapped
    bool isTapHoldMake = false;         // Whether this is a tap-hold make event
    ScanCode activeTapHoldKey = 0;      // Currently active tap-hold key (0 = none)
};

/**
 * Result of rewire processing
 */
struct RewireResult {
    VKeyCode outputKey = SC_NOP;                // Final output key
    std::vector<KeyMapEvent> eventSequence;     // Additional key events to send
    bool isModifier = false;                    // Whether output is a modifier
    ScanCode newTapHoldKey = 0;                 // New tap-hold key to remember (0 = no change, -1 = clear)
    MOD modifiersToClear = 0;                   // Modifier bitmask to clear from down state
    MOD tappedToClear = 0;                      // Modifier bitmask to clear from tapped state
    bool shouldNop = false;                     // Whether to suppress the key entirely
};

/**
 * Callback interface for querying modifier information
 */
class IModifierQuery {
public:
    virtual ~IModifierQuery() = default;
    virtual bool isModifier(VKeyCode vcode) const = 0;
    virtual MOD getModifierBitmask(VKeyCode vcode) const = 0;
};

/**
 * Pure function to apply rewire mapping.
 *
 * @param context Current key context
 * @param entry Rewire entry for this key
 * @param modQuery Interface for modifier queries (nullable)
 * @return The rewire result
 */
inline RewireResult applyRewireMapping(
    const RewireContext& context,
    const RewireEntry& entry,
    const IModifierQuery* modQuery = nullptr
) noexcept {
    RewireResult result;
    result.outputKey = context.vcode;
    
    // Ignore auto-repeating tapHold key
    if (context.activeTapHoldKey > 0 && 
        context.scancode == context.activeTapHoldKey && 
        context.isDownstroke) {
        result.outputKey = SC_NOP;
        result.shouldNop = true;
        return result;
    }
    
    // No rewire defined
    if (!entry.hasOutput()) {
        return result;
    }
    
    // Apply primary rewire
    VKeyCode rewiredKey = static_cast<VKeyCode>(entry.outKey);
    result.outputKey = rewiredKey;
    
    // Handle tapped key
    if (context.isTapped && entry.hasTap()) {
        VKeyCode tapKey = static_cast<VKeyCode>(entry.tapKey);
        
        // Clear all tapped modifiers
        result.tappedToClear = 0xFFFFFFFF;  // Clear all
        
        // Release the preceding rewired key
        result.eventSequence.push_back({rewiredKey, false});
        
        // Clear modifier down state if rewired key was a modifier
        if (modQuery != nullptr && modQuery->isModifier(rewiredKey)) {
            result.modifiersToClear = modQuery->getModifierBitmask(rewiredKey);
        }
        
        // Send tap key down and up
        result.eventSequence.push_back({tapKey, true});
        result.eventSequence.push_back({tapKey, false});
        result.outputKey = tapKey;
    }
    
    // Handle tap-hold make
    if (context.isTapHoldMake && entry.hasTapHold()) {
        if (context.activeTapHoldKey == 0) {
            VKeyCode tapHoldKey = static_cast<VKeyCode>(entry.tapHoldKey);
            result.newTapHoldKey = context.scancode;
            
            // Send make only for real keys (< 256)
            if (tapHoldKey < 256) {
                result.eventSequence.push_back({tapHoldKey, true});
            }
            result.outputKey = tapHoldKey;
            
            // Clear tapped states for the rewired key and tap key
            if (modQuery != nullptr) {
                MOD mask1 = modQuery->getModifierBitmask(rewiredKey);
                MOD mask2 = entry.hasTap() ? modQuery->getModifierBitmask(static_cast<VKeyCode>(entry.tapKey)) : 0;
                result.tappedToClear = mask1 | mask2;
            }
        }
        // Else: ignore second tap-hold (only one can be active)
    }
    
    // Handle tap-hold break
    if (!context.isDownstroke && 
        context.activeTapHoldKey > 0 &&
        context.scancode == context.activeTapHoldKey) {
        
        if (entry.hasTapHold()) {
            VKeyCode tapHoldKey = static_cast<VKeyCode>(entry.tapHoldKey);
            result.newTapHoldKey = static_cast<ScanCode>(-1);  // Signal to clear
            
            // Send break only for real keys
            if (tapHoldKey < 256) {
                result.eventSequence.push_back({tapHoldKey, false});
            } else {
                result.shouldNop = true;
            }
            result.outputKey = tapHoldKey;
        }
    }
    
    // Check if result is a modifier
    if (modQuery != nullptr) {
        result.isModifier = modQuery->isModifier(result.outputKey);
    }
    
    return result;
}


/**
 * KeyMapper class - Combines alpha and rewire mapping.
 *
 * This is an object-oriented facade over the pure functions,
 * useful when you want to encapsulate mapping tables.
 */
class KeyMapper {
public:
    // Rule of 5: Explicitly defaulted (stateless class)
    KeyMapper() noexcept = default;
    ~KeyMapper() noexcept = default;
    KeyMapper(const KeyMapper&) noexcept = default;
    KeyMapper& operator=(const KeyMapper&) noexcept = default;
    KeyMapper(KeyMapper&&) noexcept = default;
    KeyMapper& operator=(KeyMapper&&) noexcept = default;


    /**
     * Apply alpha key mapping
     */
    AlphaMapResult mapAlpha(
        VKeyCode inputKey,
        const int* alphaMap,
        const AlphaMapOptions& options,
        bool isModifierKey,
        bool isCtrlDown,
        bool isWinDown
    ) const noexcept {
        return applyAlphaMapping(inputKey, alphaMap, options, isModifierKey, isCtrlDown, isWinDown);
    }
    
    /**
     * Apply rewire mapping
     */
    RewireResult mapRewire(
        const RewireContext& context,
        const RewireEntry& entry,
        const IModifierQuery* modQuery = nullptr
    ) const noexcept {
        return applyRewireMapping(context, entry, modQuery);
    }
};

} // namespace domain
} // namespace capsicain
