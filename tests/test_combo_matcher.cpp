/**
 * test_combo_matcher.cpp
 * 
 * Unit tests for the ComboMatcher domain component.
 * Tests the pure combo matching logic extracted from processCombos().
 */

#include <gtest/gtest.h>
#include "domain/ComboMatcher.h"

using namespace capsicain::domain;

// Test constants
constexpr VKeyCode KEY_A = 0x1E;
constexpr VKeyCode KEY_B = 0x30;
constexpr VKeyCode KEY_C = 0x2E;
constexpr VKeyCode KEY_ESC = 0x01;

// Modifier bits (matching the real modifier bitmasks)
constexpr MOD MOD_LSHIFT = 0x0001;
constexpr MOD MOD_RSHIFT = 0x0002;
constexpr MOD MOD_LCTRL = 0x0004;
constexpr MOD MOD_RCTRL = 0x0008;
constexpr MOD MOD_LALT = 0x0010;
constexpr MOD MOD_RALT = 0x0020;
constexpr MOD MOD_LWIN = 0x0040;
constexpr MOD MOD_RWIN = 0x0080;

// Device bits
constexpr DEV DEV_KEYBOARD1 = 0x0001;
constexpr DEV DEV_KEYBOARD2 = 0x0002;
constexpr DEV DEV_ALL = 0xFFFF;


// ============================================================================
// Device Pattern Matching Tests
// ============================================================================

TEST(ComboMatcherTest, DevicePattern_NoRequirement_MatchesAny) {
    EXPECT_TRUE(deviceMatchesPattern(DEV_KEYBOARD1, 0, 0));
    EXPECT_TRUE(deviceMatchesPattern(DEV_KEYBOARD2, 0, 0));
    EXPECT_TRUE(deviceMatchesPattern(DEV_ALL, 0, 0));
}

TEST(ComboMatcherTest, DevicePattern_AndMask_RequiresAllBits) {
    EXPECT_TRUE(deviceMatchesPattern(DEV_KEYBOARD1, DEV_KEYBOARD1, 0));
    EXPECT_FALSE(deviceMatchesPattern(DEV_KEYBOARD1, DEV_KEYBOARD2, 0));
    EXPECT_TRUE(deviceMatchesPattern(DEV_KEYBOARD1 | DEV_KEYBOARD2, DEV_KEYBOARD1, 0));
}

TEST(ComboMatcherTest, DevicePattern_NotMask_ExcludesBits) {
    EXPECT_TRUE(deviceMatchesPattern(DEV_KEYBOARD1, 0, DEV_KEYBOARD2));
    EXPECT_FALSE(deviceMatchesPattern(DEV_KEYBOARD2, 0, DEV_KEYBOARD2));
    EXPECT_FALSE(deviceMatchesPattern(DEV_KEYBOARD1 | DEV_KEYBOARD2, 0, DEV_KEYBOARD2));
}

TEST(ComboMatcherTest, DevicePattern_Combined_AndAndNot) {
    // Must have keyboard1, must not have keyboard2
    EXPECT_TRUE(deviceMatchesPattern(DEV_KEYBOARD1, DEV_KEYBOARD1, DEV_KEYBOARD2));
    EXPECT_FALSE(deviceMatchesPattern(DEV_KEYBOARD2, DEV_KEYBOARD1, DEV_KEYBOARD2));
    EXPECT_FALSE(deviceMatchesPattern(DEV_KEYBOARD1 | DEV_KEYBOARD2, DEV_KEYBOARD1, DEV_KEYBOARD2));
}

TEST(ComboMatcherTest, DevicePattern_LegacyDefault_AllDevices) {
    // Legacy default: 0xFFFFFFFF with devNot=0 matches any device
    EXPECT_TRUE(deviceMatchesPattern(DEV_KEYBOARD1, 0xFFFFFFFF, 0));
    EXPECT_TRUE(deviceMatchesPattern(DEV_KEYBOARD2, 0xFFFFFFFF, 0));
    EXPECT_TRUE(deviceMatchesPattern(0x0001, 0xFFFFFFFF, 0));
    EXPECT_TRUE(deviceMatchesPattern(0x8000, 0xFFFFFFFF, 0));

    // But if devNot is set, the special case doesn't apply
    EXPECT_FALSE(deviceMatchesPattern(DEV_KEYBOARD1, 0xFFFFFFFF, DEV_KEYBOARD1));
}


// ============================================================================
// Modifier Pattern Matching Tests
// ============================================================================

TEST(ComboMatcherTest, ModifierPattern_NoRequirements_MatchesAny) {
    ComboRule rule;  // All zeros
    ComboMatchContext context;
    context.modifiersDown = MOD_LSHIFT | MOD_LCTRL;
    
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
}

TEST(ComboMatcherTest, ModifierPattern_AndMask_AllRequired) {
    ComboRule rule;
    rule.modAnd = MOD_LSHIFT | MOD_LCTRL;
    
    ComboMatchContext context;
    
    // Only shift - should fail
    context.modifiersDown = MOD_LSHIFT;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
    
    // Only ctrl - should fail
    context.modifiersDown = MOD_LCTRL;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
    
    // Both - should pass
    context.modifiersDown = MOD_LSHIFT | MOD_LCTRL;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
    
    // Both plus extra - should pass
    context.modifiersDown = MOD_LSHIFT | MOD_LCTRL | MOD_LALT;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
}

TEST(ComboMatcherTest, ModifierPattern_OrMask_AtLeastOne) {
    ComboRule rule;
    rule.modOr = MOD_LSHIFT | MOD_RSHIFT;
    
    ComboMatchContext context;
    
    // Neither shift - should fail
    context.modifiersDown = MOD_LCTRL;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
    
    // Left shift - should pass
    context.modifiersDown = MOD_LSHIFT;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
    
    // Right shift - should pass
    context.modifiersDown = MOD_RSHIFT;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
    
    // Both shifts - should pass
    context.modifiersDown = MOD_LSHIFT | MOD_RSHIFT;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
}

TEST(ComboMatcherTest, ModifierPattern_NotMask_ExcludesAll) {
    ComboRule rule;
    rule.modNot = MOD_LALT;
    
    ComboMatchContext context;
    
    // No alt - should pass
    context.modifiersDown = MOD_LSHIFT | MOD_LCTRL;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
    
    // Alt down - should fail
    context.modifiersDown = MOD_LALT;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
    
    // Alt with others - should fail
    context.modifiersDown = MOD_LSHIFT | MOD_LALT;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
}

TEST(ComboMatcherTest, ModifierPattern_TapMask_AllMustBeTapped) {
    ComboRule rule;
    rule.modTap = MOD_LSHIFT;
    
    ComboMatchContext context;
    
    // Not tapped - should fail
    context.modifiersTapped = 0;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
    
    // Tapped - should pass
    context.modifiersTapped = MOD_LSHIFT;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
    
    // Tapped with extras - should pass
    context.modifiersTapped = MOD_LSHIFT | MOD_LCTRL;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
}

TEST(ComboMatcherTest, ModifierPattern_TapAndMask_TappedOrDown) {
    ComboRule rule;
    rule.modTapAnd = MOD_LSHIFT;
    
    ComboMatchContext context;
    
    // Neither tapped nor down - should fail
    context.modifiersDown = 0;
    context.modifiersTapped = 0;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
    
    // Tapped but not down - should pass
    context.modifiersDown = 0;
    context.modifiersTapped = MOD_LSHIFT;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
    
    // Down but not tapped - should pass
    context.modifiersDown = MOD_LSHIFT;
    context.modifiersTapped = 0;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
    
    // Both tapped and down - should pass
    context.modifiersDown = MOD_LSHIFT;
    context.modifiersTapped = MOD_LSHIFT;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
}

TEST(ComboMatcherTest, ModifierPattern_Deadkey_MustMatch) {
    ComboRule rule;
    rule.deadkey = 1;
    
    ComboMatchContext context;
    
    // Wrong deadkey - should fail
    context.activeDeadkey = 0;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
    
    context.activeDeadkey = 2;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
    
    // Correct deadkey - should pass
    context.activeDeadkey = 1;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
}

TEST(ComboMatcherTest, ModifierPattern_Combined_ComplexPattern) {
    // Ctrl+Shift+A but not Alt
    ComboRule rule;
    rule.modAnd = MOD_LSHIFT | MOD_LCTRL;
    rule.modNot = MOD_LALT;
    
    ComboMatchContext context;
    
    // Missing ctrl - should fail
    context.modifiersDown = MOD_LSHIFT;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
    
    // Has all required - should pass
    context.modifiersDown = MOD_LSHIFT | MOD_LCTRL;
    EXPECT_TRUE(modifiersMatchPattern(context, rule));
    
    // Has alt - should fail
    context.modifiersDown = MOD_LSHIFT | MOD_LCTRL | MOD_LALT;
    EXPECT_FALSE(modifiersMatchPattern(context, rule));
}


// ============================================================================
// Combo Finding Tests
// ============================================================================

TEST(ComboMatcherTest, FindCombo_EmptyList_NoMatch) {
    std::vector<ComboRule> combos;
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    ComboMatchResult result = findMatchingCombo(combos, context);
    EXPECT_FALSE(result.matched);
}

TEST(ComboMatcherTest, FindCombo_WrongKey_NoMatch) {
    ComboRule rule;
    rule.triggerKey = KEY_B;
    rule.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> combos = {rule};
    ComboMatchContext context;
    context.currentKey = KEY_A;  // Different key
    
    ComboMatchResult result = findMatchingCombo(combos, context);
    EXPECT_FALSE(result.matched);
}

TEST(ComboMatcherTest, FindCombo_SimpleMatch) {
    ComboRule rule;
    rule.triggerKey = KEY_A;
    rule.resultSequence = {{KEY_B, true}, {KEY_B, false}};
    
    std::vector<ComboRule> combos = {rule};
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    ComboMatchResult result = findMatchingCombo(combos, context);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.matchedIndex, 0u);
    ASSERT_EQ(result.resultSequence.size(), 2u);
    EXPECT_EQ(result.resultSequence[0].keyCode, KEY_B);
    EXPECT_TRUE(result.resultSequence[0].isDown);
    EXPECT_EQ(result.resultSequence[1].keyCode, KEY_B);
    EXPECT_FALSE(result.resultSequence[1].isDown);
}

TEST(ComboMatcherTest, FindCombo_MatchWithModifiers) {
    ComboRule rule;
    rule.triggerKey = KEY_A;
    rule.modAnd = MOD_LCTRL;
    rule.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> combos = {rule};
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    // Without modifier - no match
    context.modifiersDown = 0;
    ComboMatchResult result = findMatchingCombo(combos, context);
    EXPECT_FALSE(result.matched);
    
    // With modifier - match
    context.modifiersDown = MOD_LCTRL;
    result = findMatchingCombo(combos, context);
    EXPECT_TRUE(result.matched);
}

TEST(ComboMatcherTest, FindCombo_FirstMatchWins) {
    ComboRule rule1;
    rule1.triggerKey = KEY_A;
    rule1.resultSequence = {{KEY_B, true}};
    
    ComboRule rule2;
    rule2.triggerKey = KEY_A;
    rule2.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> combos = {rule1, rule2};
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    ComboMatchResult result = findMatchingCombo(combos, context);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.matchedIndex, 0u);
    EXPECT_EQ(result.resultSequence[0].keyCode, KEY_B);  // First rule's result
}

TEST(ComboMatcherTest, FindCombo_SkipsNonMatchingRules) {
    ComboRule rule1;
    rule1.triggerKey = KEY_A;
    rule1.modAnd = MOD_LSHIFT;  // Requires shift
    rule1.resultSequence = {{KEY_B, true}};
    
    ComboRule rule2;
    rule2.triggerKey = KEY_A;
    // No modifier requirement
    rule2.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> combos = {rule1, rule2};
    ComboMatchContext context;
    context.currentKey = KEY_A;
    context.modifiersDown = 0;  // No shift held
    
    ComboMatchResult result = findMatchingCombo(combos, context);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.matchedIndex, 1u);  // Second rule matched
    EXPECT_EQ(result.resultSequence[0].keyCode, KEY_C);
}

TEST(ComboMatcherTest, FindCombo_ClearTappedFlag) {
    ComboRule rule;
    rule.triggerKey = KEY_A;
    rule.resultSequence = {{KEY_B, true}};
    
    std::vector<ComboRule> combos = {rule};
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    // Without clearTapped flag
    ComboMatchResult result = findMatchingCombo(combos, context, false);
    EXPECT_TRUE(result.matched);
    EXPECT_FALSE(result.shouldClearTapped);
    
    // With clearTapped flag
    result = findMatchingCombo(combos, context, true);
    EXPECT_TRUE(result.matched);
    EXPECT_TRUE(result.shouldClearTapped);
}


// ============================================================================
// ComboMatcher Class Tests
// ============================================================================

TEST(ComboMatcherTest, MatchDownstroke_RegularDown) {
    ComboMatcher matcher;
    
    ComboRule rule;
    rule.triggerKey = KEY_A;
    rule.resultSequence = {{KEY_B, true}};
    
    std::vector<ComboRule> downCombos = {rule};
    std::vector<ComboRule> repeatCombos;
    
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    ComboMatchResult result = matcher.matchDownstroke(downCombos, repeatCombos, context, false);
    EXPECT_TRUE(result.matched);
    EXPECT_TRUE(result.shouldClearTapped);  // Down combos clear tapped
}

TEST(ComboMatcherTest, MatchDownstroke_Repeat_TriesBoth) {
    ComboMatcher matcher;
    
    ComboRule repeatRule;
    repeatRule.triggerKey = KEY_A;
    repeatRule.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> downCombos;  // Empty
    std::vector<ComboRule> repeatCombos = {repeatRule};
    
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    // Not repeat - no match (down combos empty)
    ComboMatchResult result = matcher.matchDownstroke(downCombos, repeatCombos, context, false);
    EXPECT_FALSE(result.matched);
    
    // Is repeat - matches repeat combo
    result = matcher.matchDownstroke(downCombos, repeatCombos, context, true);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.resultSequence[0].keyCode, KEY_C);
}

TEST(ComboMatcherTest, MatchDownstroke_DownComboTakesPriority) {
    ComboMatcher matcher;
    
    ComboRule downRule;
    downRule.triggerKey = KEY_A;
    downRule.resultSequence = {{KEY_B, true}};
    
    ComboRule repeatRule;
    repeatRule.triggerKey = KEY_A;
    repeatRule.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> downCombos = {downRule};
    std::vector<ComboRule> repeatCombos = {repeatRule};
    
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    // Even with repeat, down combo wins
    ComboMatchResult result = matcher.matchDownstroke(downCombos, repeatCombos, context, true);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.resultSequence[0].keyCode, KEY_B);
}

TEST(ComboMatcherTest, MatchUpstroke_RegularUp) {
    ComboMatcher matcher;
    
    ComboRule rule;
    rule.triggerKey = KEY_A;
    rule.resultSequence = {{KEY_B, false}};
    
    std::vector<ComboRule> upCombos = {rule};
    std::vector<ComboRule> tapCombos;
    std::vector<ComboRule> slowCombos;
    
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    ComboMatchResult result = matcher.matchUpstroke(upCombos, tapCombos, slowCombos, context, false, false);
    EXPECT_TRUE(result.matched);
}

TEST(ComboMatcherTest, MatchUpstroke_TapCombo) {
    ComboMatcher matcher;
    
    ComboRule tapRule;
    tapRule.triggerKey = KEY_A;
    tapRule.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> upCombos;
    std::vector<ComboRule> tapCombos = {tapRule};
    std::vector<ComboRule> slowCombos;
    
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    // Not tapped - no match
    ComboMatchResult result = matcher.matchUpstroke(upCombos, tapCombos, slowCombos, context, false, false);
    EXPECT_FALSE(result.matched);
    
    // Tapped - matches
    result = matcher.matchUpstroke(upCombos, tapCombos, slowCombos, context, true, false);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.resultSequence[0].keyCode, KEY_C);
}

TEST(ComboMatcherTest, MatchUpstroke_SlowCombo) {
    ComboMatcher matcher;
    
    ComboRule slowRule;
    slowRule.triggerKey = KEY_A;
    slowRule.resultSequence = {{KEY_B, true}};
    
    std::vector<ComboRule> upCombos;
    std::vector<ComboRule> tapCombos;
    std::vector<ComboRule> slowCombos = {slowRule};
    
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    // Not slow tapped - no match
    ComboMatchResult result = matcher.matchUpstroke(upCombos, tapCombos, slowCombos, context, false, false);
    EXPECT_FALSE(result.matched);
    
    // Slow tapped - matches
    result = matcher.matchUpstroke(upCombos, tapCombos, slowCombos, context, false, true);
    EXPECT_TRUE(result.matched);
}

TEST(ComboMatcherTest, MatchUpstroke_UpComboTakesPriority) {
    ComboMatcher matcher;
    
    ComboRule upRule;
    upRule.triggerKey = KEY_A;
    upRule.resultSequence = {{KEY_B, false}};
    
    ComboRule tapRule;
    tapRule.triggerKey = KEY_A;
    tapRule.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> upCombos = {upRule};
    std::vector<ComboRule> tapCombos = {tapRule};
    std::vector<ComboRule> slowCombos;
    
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    // Even with tap, up combo wins
    ComboMatchResult result = matcher.matchUpstroke(upCombos, tapCombos, slowCombos, context, true, false);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.resultSequence[0].keyCode, KEY_B);
}

TEST(ComboMatcherTest, MatchUpstroke_SlowBeforeTap) {
    ComboMatcher matcher;
    
    ComboRule slowRule;
    slowRule.triggerKey = KEY_A;
    slowRule.resultSequence = {{KEY_B, true}};
    
    ComboRule tapRule;
    tapRule.triggerKey = KEY_A;
    tapRule.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> upCombos;
    std::vector<ComboRule> tapCombos = {tapRule};
    std::vector<ComboRule> slowCombos = {slowRule};
    
    ComboMatchContext context;
    context.currentKey = KEY_A;
    
    // Both slow and tap - slow takes priority
    ComboMatchResult result = matcher.matchUpstroke(upCombos, tapCombos, slowCombos, context, true, true);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.resultSequence[0].keyCode, KEY_B);  // Slow rule's result
}
