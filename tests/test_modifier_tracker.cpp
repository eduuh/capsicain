/**
 * @file test_modifier_tracker.cpp
 * @brief Unit tests for ModifierTracker
 */

#include <gtest/gtest.h>
#include "domain/ModifierTracker.h"
#include "scancodes.h"

using namespace capsicain;

class ModifierTrackerTest : public ::testing::Test {
protected:
    ModifierTracker tracker;
    
    void SetUp() override {
        tracker.reset();
    }
};

//==============================================================================
// Basic update tests
//==============================================================================

TEST_F(ModifierTrackerTest, InitialState_AllClear) {
    EXPECT_FALSE(tracker.isShiftDown());
    EXPECT_FALSE(tracker.isCtrlDown());
    EXPECT_FALSE(tracker.isAltDown());
    EXPECT_FALSE(tracker.isWinDown());
    EXPECT_EQ(tracker.getDownMask(), 0);
    EXPECT_EQ(tracker.getTappedMask(), 0);
}

TEST_F(ModifierTrackerTest, Update_LShiftDown) {
    tracker.update(SC_LSHIFT, true, false);
    
    EXPECT_TRUE(tracker.isShiftDown());
    EXPECT_TRUE(tracker.isLShiftDown());
    EXPECT_FALSE(tracker.isRShiftDown());
}

TEST_F(ModifierTrackerTest, Update_LShiftUp) {
    tracker.update(SC_LSHIFT, true, false);  // Press
    tracker.update(SC_LSHIFT, false, false); // Release
    
    EXPECT_FALSE(tracker.isShiftDown());
    EXPECT_FALSE(tracker.isLShiftDown());
}

TEST_F(ModifierTrackerTest, Update_BothShifts) {
    tracker.update(SC_LSHIFT, true, false);
    tracker.update(SC_RSHIFT, true, false);
    
    EXPECT_TRUE(tracker.isShiftDown());
    EXPECT_TRUE(tracker.isLShiftDown());
    EXPECT_TRUE(tracker.isRShiftDown());
    
    tracker.update(SC_LSHIFT, false, false);
    
    EXPECT_TRUE(tracker.isShiftDown());   // Still down (RSHIFT)
    EXPECT_FALSE(tracker.isLShiftDown());
    EXPECT_TRUE(tracker.isRShiftDown());
}

TEST_F(ModifierTrackerTest, Update_AllModifiers) {
    tracker.update(SC_LSHIFT, true, false);
    tracker.update(SC_LCTRL, true, false);
    tracker.update(SC_LALT, true, false);
    tracker.update(SC_LWIN, true, false);
    
    EXPECT_TRUE(tracker.isShiftDown());
    EXPECT_TRUE(tracker.isCtrlDown());
    EXPECT_TRUE(tracker.isAltDown());
    EXPECT_TRUE(tracker.isWinDown());
}

TEST_F(ModifierTrackerTest, Update_NonModifier_NoEffect) {
    tracker.update(SC_A, true, false);
    
    EXPECT_FALSE(tracker.isShiftDown());
    EXPECT_FALSE(tracker.isCtrlDown());
    EXPECT_EQ(tracker.getDownMask(), 0);
}

//==============================================================================
// Tapped state tests
//==============================================================================

TEST_F(ModifierTrackerTest, Tapped_SetsTappedMask) {
    tracker.update(SC_LSHIFT, true, false);  // Press
    tracker.update(SC_LSHIFT, false, true);  // Release with tap
    
    EXPECT_TRUE(tracker.isModifierTapped(SC_LSHIFT));
    EXPECT_NE(tracker.getTappedMask(), 0);
}

TEST_F(ModifierTrackerTest, Tapped_MultipleModifiers) {
    // Tap Ctrl, then Tap Alt
    tracker.update(SC_LCTRL, true, false);
    tracker.update(SC_LCTRL, false, true);
    
    tracker.update(SC_LALT, true, false);
    tracker.update(SC_LALT, false, true);
    
    EXPECT_TRUE(tracker.isModifierTapped(SC_LCTRL));
    EXPECT_TRUE(tracker.isModifierTapped(SC_LALT));
}

TEST_F(ModifierTrackerTest, ClearTapped_Single) {
    tracker.update(SC_LSHIFT, false, true);  // Tap
    tracker.clearTapped(SC_LSHIFT);
    
    EXPECT_FALSE(tracker.isModifierTapped(SC_LSHIFT));
}

TEST_F(ModifierTrackerTest, ClearAllTapped) {
    tracker.update(SC_LSHIFT, false, true);
    tracker.update(SC_LCTRL, false, true);
    
    tracker.clearAllTapped();
    
    EXPECT_FALSE(tracker.isModifierTapped(SC_LSHIFT));
    EXPECT_FALSE(tracker.isModifierTapped(SC_LCTRL));
    EXPECT_EQ(tracker.getTappedMask(), 0);
}

//==============================================================================
// Forced modifier tests
//==============================================================================

TEST_F(ModifierTrackerTest, ForceDown_SetsDownState) {
    tracker.forceDown(SC_LSHIFT);
    
    EXPECT_TRUE(tracker.isShiftDown());
    EXPECT_TRUE(tracker.isLShiftDown());
}

TEST_F(ModifierTrackerTest, ForceDown_PersistsAfterRelease) {
    tracker.forceDown(SC_LSHIFT);
    tracker.update(SC_LSHIFT, false, false);
    
    // Force is still active, so update reapplies it
    tracker.update(SC_A, true, false);  // Any key triggers update
    
    EXPECT_NE(tracker.getForcedMask(), 0);
}

TEST_F(ModifierTrackerTest, ClearForced) {
    tracker.forceDown(SC_LSHIFT);
    tracker.clearForced(SC_LSHIFT);
    
    EXPECT_EQ(tracker.getForcedMask(), 0);
}

//==============================================================================
// Pattern matching tests
//==============================================================================

TEST_F(ModifierTrackerTest, MatchesPattern_AndMask) {
    tracker.update(SC_LSHIFT, true, false);
    tracker.update(SC_LCTRL, true, false);
    
    // Both Shift and Ctrl must be down
    EXPECT_TRUE(tracker.matchesPattern(BITMASK_LSHIFT | BITMASK_LCTRL, 0, 0));
    
    // Only Shift required - passes
    EXPECT_TRUE(tracker.matchesPattern(BITMASK_LSHIFT, 0, 0));
    
    // Alt required - fails
    EXPECT_FALSE(tracker.matchesPattern(BITMASK_LALT, 0, 0));
}

TEST_F(ModifierTrackerTest, MatchesPattern_OrMask) {
    tracker.update(SC_LSHIFT, true, false);
    
    // Either Shift or Ctrl must be down - passes (Shift is down)
    EXPECT_TRUE(tracker.matchesPattern(0, BITMASK_LSHIFT | BITMASK_LCTRL, 0));
    
    // Either Alt or Win - fails (neither is down)
    EXPECT_FALSE(tracker.matchesPattern(0, BITMASK_LALT | BITMASK_LWIN, 0));
}

TEST_F(ModifierTrackerTest, MatchesPattern_NotMask) {
    tracker.update(SC_LSHIFT, true, false);
    
    // Shift must NOT be down - fails
    EXPECT_FALSE(tracker.matchesPattern(0, 0, BITMASK_LSHIFT));
    
    // Alt must NOT be down - passes (Alt is not down)
    EXPECT_TRUE(tracker.matchesPattern(0, 0, BITMASK_LALT));
}

TEST_F(ModifierTrackerTest, MatchesPattern_Combined) {
    tracker.update(SC_LSHIFT, true, false);
    tracker.update(SC_LCTRL, true, false);
    
    // Shift AND Ctrl must be down, Alt must NOT be down
    EXPECT_TRUE(tracker.matchesPattern(
        BITMASK_LSHIFT | BITMASK_LCTRL,  // AND
        0,                                 // OR
        BITMASK_LALT                      // NOT
    ));
    
    // Add Alt - now should fail
    tracker.update(SC_LALT, true, false);
    EXPECT_FALSE(tracker.matchesPattern(
        BITMASK_LSHIFT | BITMASK_LCTRL,
        0,
        BITMASK_LALT
    ));
}

TEST_F(ModifierTrackerTest, MatchesTappedPattern) {
    tracker.update(SC_LSHIFT, false, true);  // Tap shift
    tracker.update(SC_LCTRL, false, true);   // Tap ctrl
    
    EXPECT_TRUE(tracker.matchesTappedPattern(BITMASK_LSHIFT));
    EXPECT_TRUE(tracker.matchesTappedPattern(BITMASK_LCTRL));
    EXPECT_TRUE(tracker.matchesTappedPattern(BITMASK_LSHIFT | BITMASK_LCTRL));
    EXPECT_FALSE(tracker.matchesTappedPattern(BITMASK_LALT));
}

//==============================================================================
// isModifierDown / isModifierTapped tests
//==============================================================================

TEST_F(ModifierTrackerTest, IsModifierDown_ByVcode) {
    tracker.update(SC_LCTRL, true, false);
    
    EXPECT_TRUE(tracker.isModifierDown(SC_LCTRL));
    EXPECT_FALSE(tracker.isModifierDown(SC_RCTRL));
    EXPECT_FALSE(tracker.isModifierDown(SC_A));  // Non-modifier
}

TEST_F(ModifierTrackerTest, IsModifierTapped_ByVcode) {
    tracker.update(SC_LALT, false, true);
    
    EXPECT_TRUE(tracker.isModifierTapped(SC_LALT));
    EXPECT_FALSE(tracker.isModifierTapped(SC_RALT));
}

//==============================================================================
// Deadkey tests
//==============================================================================

TEST_F(ModifierTrackerTest, Deadkey_SetAndGet) {
    tracker.setDeadkey('G');  // Grave
    
    EXPECT_EQ(tracker.getDeadkey(), 'G');
}

TEST_F(ModifierTrackerTest, Deadkey_Clear) {
    tracker.setDeadkey('G');
    tracker.clearDeadkey();
    
    EXPECT_EQ(tracker.getDeadkey(), 0);
}

//==============================================================================
// Tap-and-hold key tests
//==============================================================================

TEST_F(ModifierTrackerTest, TapHoldKey_SetAndGet) {
    EXPECT_FALSE(tracker.hasTapHoldKey());
    EXPECT_EQ(tracker.getTapHoldKey(), -1);
    
    tracker.setTapHoldKey(SC_TAB);
    
    EXPECT_TRUE(tracker.hasTapHoldKey());
    EXPECT_EQ(tracker.getTapHoldKey(), SC_TAB);
}

TEST_F(ModifierTrackerTest, TapHoldKey_Clear) {
    tracker.setTapHoldKey(SC_TAB);
    tracker.clearTapHoldKey();
    
    EXPECT_FALSE(tracker.hasTapHoldKey());
    EXPECT_EQ(tracker.getTapHoldKey(), -1);
}

//==============================================================================
// Reset tests
//==============================================================================

TEST_F(ModifierTrackerTest, Reset_ClearsEverything) {
    tracker.update(SC_LSHIFT, true, false);
    tracker.update(SC_LCTRL, false, true);
    tracker.forceDown(SC_LALT);
    tracker.setDeadkey('G');
    tracker.setTapHoldKey(SC_TAB);
    
    tracker.reset();
    
    EXPECT_EQ(tracker.getDownMask(), 0);
    EXPECT_EQ(tracker.getTappedMask(), 0);
    EXPECT_EQ(tracker.getForcedMask(), 0);
    EXPECT_EQ(tracker.getDeadkey(), 0);
    EXPECT_FALSE(tracker.hasTapHoldKey());
}

//==============================================================================
// Virtual modifier tests
//==============================================================================

TEST_F(ModifierTrackerTest, VirtualModifier_MOD9) {
    tracker.update(VK_MOD9, true, false);
    
    // Virtual modifiers should work like real ones
    EXPECT_TRUE(tracker.isModifierDown(VK_MOD9));
    EXPECT_NE(tracker.getDownMask() & BITMASK_MOD9, 0);
}

TEST_F(ModifierTrackerTest, VirtualModifier_Tapped) {
    tracker.update(VK_MOD10, false, true);
    
    EXPECT_TRUE(tracker.isModifierTapped(VK_MOD10));
}
