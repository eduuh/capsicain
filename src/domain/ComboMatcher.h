#pragma once
/**
 * ComboMatcher.h
 * 
 * Pure domain logic for matching key combinations against modifier patterns.
 * Extracted from the processCombos() function in capsicain.cpp for testability.
 * 
 * A combo matches when:
 * - The key being pressed matches the combo's trigger key
 * - The device passes the device mask check
 * - The required modifiers (AND mask) are all held down
 * - At least one optional modifier (OR mask) is held down (if any are specified)
 * - None of the excluded modifiers (NOT mask) are held down
 * - The required tapped modifiers have been tapped
 * - The deadkey matches (if specified)
 */

#include <vector>
#include <cstdint>

namespace capsicain {
namespace domain {

using MOD = uint32_t;
using DEV = uint32_t;  // Device mask (matches platform definition)
using VKeyCode = uint16_t;

/**
 * Represents a single key event (key + direction)
 */
struct ComboKeyEvent {
    VKeyCode keyCode = 0;
    bool isDown = true;

    constexpr ComboKeyEvent() noexcept = default;
    constexpr ComboKeyEvent(VKeyCode k, bool d) noexcept : keyCode(k), isDown(d) {}

    constexpr bool operator==(const ComboKeyEvent& other) const noexcept {
        return keyCode == other.keyCode && isDown == other.isDown;
    }
};

/**
 * Represents a modifier combination rule.
 * This is a clean version of the legacy ModifierCombo struct.
 */
struct ComboRule {
    VKeyCode triggerKey = 0;        // The key that triggers this combo
    
    // Modifier patterns
    MOD modAnd = 0;                 // All these modifiers must be down
    MOD modOr = 0;                  // At least one of these must be down (0 = no requirement)
    MOD modNot = 0;                 // None of these can be down
    MOD modTap = 0;                 // All these must have been tapped
    MOD modTapAnd = 0;              // All these must be tapped OR down
    
    // Device patterns
    DEV devAnd = 0;                 // Device must match this mask
    DEV devNot = 0;                 // Device must not match this mask
    
    // Deadkey requirement
    uint8_t deadkey = 0;            // Active deadkey must match (0 = no requirement)
    
    // Result when combo matches
    std::vector<ComboKeyEvent> resultSequence;  // Keys to output
};

/**
 * Current state snapshot for combo matching
 */
struct ComboMatchContext {
    VKeyCode currentKey = 0;        // The key being pressed/released
    MOD modifiersDown = 0;          // Bitmask of modifiers currently held
    MOD modifiersTapped = 0;        // Bitmask of modifiers that were tapped
    DEV deviceMask = 0;             // Current device identifier
    uint8_t activeDeadkey = 0;      // Currently active deadkey (0 = none)
};

/**
 * Result of combo matching
 */
struct ComboMatchResult {
    bool matched = false;                           // Whether a combo was found
    std::vector<ComboKeyEvent> resultSequence;      // Keys to output (empty if no match)
    size_t matchedIndex = 0;                        // Index of matched combo in the list
    bool shouldClearTapped = false;                 // Whether to clear tapped state
};

/**
 * Pure function to check if a device matches the device pattern.
 *
 * @param deviceMask Current device identifier
 * @param devAnd Required device bits (all must match)
 * @param devNot Excluded device bits (none can match)
 * @return true if device passes the check
 */
constexpr inline bool deviceMatchesPattern(DEV deviceMask, DEV devAnd, DEV devNot) noexcept {
    // Special case: 0xFFFFFFFF with no exclusions matches any device (legacy default)
    if (devAnd == 0xFFFFFFFF && devNot == 0) {
        return true;
    }

    // If devAnd is 0, any device matches
    // Otherwise, all bits in devAnd must be present in deviceMask
    bool andMatches = (devAnd == 0) || ((deviceMask & devAnd) == devAnd);

    // None of the bits in devNot can be present in deviceMask
    bool notMatches = (deviceMask & devNot) == 0;

    return andMatches && notMatches;
}

/**
 * Pure function to check if current modifier state matches a combo's pattern.
 *
 * @param context Current state snapshot
 * @param rule The combo rule to test against
 * @return true if modifiers match the pattern
 */
constexpr inline bool modifiersMatchPattern(const ComboMatchContext& context, const ComboRule& rule) noexcept {
    // Deadkey must match
    if (context.activeDeadkey != rule.deadkey) {
        return false;
    }
    
    // AND: All required modifiers must be down
    if ((context.modifiersDown & rule.modAnd) != rule.modAnd) {
        return false;
    }
    
    // OR: At least one must be down (if any specified)
    if (rule.modOr != 0 && (context.modifiersDown & rule.modOr) == 0) {
        return false;
    }
    
    // NOT: None of these can be down
    if ((context.modifiersDown & rule.modNot) != 0) {
        return false;
    }
    
    // TAP: All tapped modifiers must have been tapped
    if ((context.modifiersTapped & rule.modTap) != rule.modTap) {
        return false;
    }
    
    // TAP-AND: All must be either tapped OR currently down
    if (rule.modTapAnd != 0) {
        bool tappedMatch = (context.modifiersTapped & rule.modTapAnd) == rule.modTapAnd;
        bool downMatch = (context.modifiersDown & rule.modTapAnd) == rule.modTapAnd;
        if (!tappedMatch && !downMatch) {
            return false;
        }
    }
    
    return true;
}

/**
 * Pure function to find a matching combo from a list.
 *
 * @param combos List of combo rules to search
 * @param context Current state snapshot
 * @param clearTappedOnMatch Whether to set shouldClearTapped in result
 * @return Match result with matched combo info
 */
inline ComboMatchResult findMatchingCombo(
    const std::vector<ComboRule>& combos,
    const ComboMatchContext& context,
    bool clearTappedOnMatch = false
) noexcept {
    ComboMatchResult result;
    
    for (size_t i = 0; i < combos.size(); ++i) {
        const ComboRule& rule = combos[i];
        
        // Key must match
        if (rule.triggerKey != context.currentKey) {
            continue;
        }
        
        // Device must match pattern
        if (!deviceMatchesPattern(context.deviceMask, rule.devAnd, rule.devNot)) {
            continue;
        }
        
        // Modifiers must match pattern
        if (!modifiersMatchPattern(context, rule)) {
            continue;
        }
        
        // Match found!
        result.matched = true;
        result.resultSequence = rule.resultSequence;
        result.matchedIndex = i;
        result.shouldClearTapped = clearTappedOnMatch;
        return result;
    }
    
    return result;  // No match
}

/**
 * ComboMatcher class - Stateless combo matching engine.
 *
 * This class provides an object-oriented interface around the pure functions
 * above, useful for scenarios where you want to encapsulate combo list management.
 */
class ComboMatcher {
public:
    // Rule of 5: Explicitly defaulted (stateless class)
    ComboMatcher() noexcept = default;
    ~ComboMatcher() noexcept = default;
    ComboMatcher(const ComboMatcher&) noexcept = default;
    ComboMatcher& operator=(const ComboMatcher&) noexcept = default;
    ComboMatcher(ComboMatcher&&) noexcept = default;
    ComboMatcher& operator=(ComboMatcher&&) noexcept = default;


    /**
     * Find a matching combo for a downstroke.
     */
    ComboMatchResult matchDownstroke(
        const std::vector<ComboRule>& downCombos,
        const std::vector<ComboRule>& repeatCombos,
        const ComboMatchContext& context,
        bool isRepeat
    ) const noexcept {
        // First try regular down combos (clears tapped on match)
        ComboMatchResult result = findMatchingCombo(downCombos, context, true);
        if (result.matched) {
            return result;
        }
        
        // If repeating, also try repeat combos
        if (isRepeat) {
            result = findMatchingCombo(repeatCombos, context, false);
        }
        
        return result;
    }
    
    /**
     * Find a matching combo for an upstroke.
     */
    ComboMatchResult matchUpstroke(
        const std::vector<ComboRule>& upCombos,
        const std::vector<ComboRule>& tapCombos,
        const std::vector<ComboRule>& slowCombos,
        const ComboMatchContext& context,
        bool isTapped,
        bool isSlowTapped
    ) const noexcept {
        // Try regular up combos
        ComboMatchResult result = findMatchingCombo(upCombos, context, false);
        if (result.matched) {
            return result;
        }
        
        // If slow-tapped, try slow combos
        if (isSlowTapped) {
            result = findMatchingCombo(slowCombos, context, false);
            if (result.matched) {
                return result;
            }
        }
        
        // If tapped, try tap combos
        if (isTapped) {
            result = findMatchingCombo(tapCombos, context, false);
        }
        
        return result;
    }
};

} // namespace domain
} // namespace capsicain
