/**
 * @file test_modifiers.cpp
 * @brief Unit tests for modifiers.cpp functions
 * 
 * Tests modifier bitmask operations and modifier type checking.
 */

#include <gtest/gtest.h>
#include "legacy/scancodes.h"
#include "legacy/modifiers.h"

//==============================================================================
// getModifierBitmaskForVcode tests
//==============================================================================

TEST(Modifiers, BitmaskForVcode_LeftModifiers) {
    EXPECT_EQ(getModifierBitmaskForVcode(SC_LSHIFT), BITMASK_LSHIFT);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_LCTRL), BITMASK_LCTRL);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_LALT), BITMASK_LALT);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_LWIN), BITMASK_LWIN);
}

TEST(Modifiers, BitmaskForVcode_RightModifiers) {
    EXPECT_EQ(getModifierBitmaskForVcode(SC_RSHIFT), BITMASK_RSHIFT);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_RCTRL), BITMASK_RCTRL);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_RALT), BITMASK_RALT);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_RWIN), BITMASK_RWIN);
}

TEST(Modifiers, BitmaskForVcode_VirtualModifiers) {
    EXPECT_EQ(getModifierBitmaskForVcode(VK_MOD9), BITMASK_MOD9);
    EXPECT_EQ(getModifierBitmaskForVcode(VK_MOD10), BITMASK_MOD10);
    EXPECT_EQ(getModifierBitmaskForVcode(VK_MOD15), BITMASK_MOD15);
    EXPECT_EQ(getModifierBitmaskForVcode(VK_MOD16), BITMASK_MOD16);
}

TEST(Modifiers, BitmaskForVcode_NonModifier) {
    EXPECT_EQ(getModifierBitmaskForVcode(SC_A), 0);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_SPACE), 0);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_ESCAPE), 0);
    EXPECT_EQ(getModifierBitmaskForVcode(SC_RETURN), 0);
}

TEST(Modifiers, BitmaskForVcode_InvalidVcode) {
    EXPECT_EQ(getModifierBitmaskForVcode(-1), 0);
    EXPECT_EQ(getModifierBitmaskForVcode(-100), 0);
}

//==============================================================================
// getModifierForBitmask tests
//==============================================================================

TEST(Modifiers, ModifierForBitmask_LeftModifiers) {
    EXPECT_EQ(getModifierForBitmask(BITMASK_LSHIFT), SC_LSHIFT);
    EXPECT_EQ(getModifierForBitmask(BITMASK_LCTRL), SC_LCTRL);
    EXPECT_EQ(getModifierForBitmask(BITMASK_LALT), SC_LALT);
    EXPECT_EQ(getModifierForBitmask(BITMASK_LWIN), SC_LWIN);
}

TEST(Modifiers, ModifierForBitmask_RightModifiers) {
    EXPECT_EQ(getModifierForBitmask(BITMASK_RSHIFT), SC_RSHIFT);
    EXPECT_EQ(getModifierForBitmask(BITMASK_RCTRL), SC_RCTRL);
    EXPECT_EQ(getModifierForBitmask(BITMASK_RALT), SC_RALT);
    EXPECT_EQ(getModifierForBitmask(BITMASK_RWIN), SC_RWIN);
}

TEST(Modifiers, ModifierForBitmask_VirtualModifiers) {
    EXPECT_EQ(getModifierForBitmask(BITMASK_MOD9), VK_MOD9);
    EXPECT_EQ(getModifierForBitmask(BITMASK_MOD10), VK_MOD10);
}

TEST(Modifiers, ModifierForBitmask_InvalidBitmask) {
    EXPECT_EQ(getModifierForBitmask(0), 0);
}

//==============================================================================
// isModifier tests
//==============================================================================

TEST(Modifiers, IsModifier_RealModifiers) {
    EXPECT_TRUE(isModifier(SC_LSHIFT));
    EXPECT_TRUE(isModifier(SC_RSHIFT));
    EXPECT_TRUE(isModifier(SC_LCTRL));
    EXPECT_TRUE(isModifier(SC_RCTRL));
    EXPECT_TRUE(isModifier(SC_LALT));
    EXPECT_TRUE(isModifier(SC_RALT));
    EXPECT_TRUE(isModifier(SC_LWIN));
    EXPECT_TRUE(isModifier(SC_RWIN));
}

TEST(Modifiers, IsModifier_VirtualModifiers) {
    EXPECT_TRUE(isModifier(VK_MOD9));
    EXPECT_TRUE(isModifier(VK_MOD10));
    EXPECT_TRUE(isModifier(VK_MOD15));
    EXPECT_TRUE(isModifier(VK_MOD32));
}

TEST(Modifiers, IsModifier_NonModifiers) {
    EXPECT_FALSE(isModifier(SC_A));
    EXPECT_FALSE(isModifier(SC_SPACE));
    EXPECT_FALSE(isModifier(SC_RETURN));
    EXPECT_FALSE(isModifier(SC_ESCAPE));
    EXPECT_FALSE(isModifier(SC_TAB));
}

TEST(Modifiers, IsModifier_InvalidVcode) {
    EXPECT_FALSE(isModifier(-1));
    EXPECT_FALSE(isModifier(-100));
}

//==============================================================================
// isRealModifier tests
//==============================================================================

TEST(Modifiers, IsRealModifier_RealModifiers) {
    EXPECT_TRUE(isRealModifier(SC_LSHIFT));
    EXPECT_TRUE(isRealModifier(SC_RSHIFT));
    EXPECT_TRUE(isRealModifier(SC_LCTRL));
    EXPECT_TRUE(isRealModifier(SC_RCTRL));
    EXPECT_TRUE(isRealModifier(SC_LALT));
    EXPECT_TRUE(isRealModifier(SC_RALT));
    EXPECT_TRUE(isRealModifier(SC_LWIN));
    EXPECT_TRUE(isRealModifier(SC_RWIN));
}

TEST(Modifiers, IsRealModifier_VirtualModifiersAreFalse) {
    EXPECT_FALSE(isRealModifier(VK_MOD9));
    EXPECT_FALSE(isRealModifier(VK_MOD10));
    EXPECT_FALSE(isRealModifier(VK_MOD15));
}

TEST(Modifiers, IsRealModifier_NonModifiers) {
    EXPECT_FALSE(isRealModifier(SC_A));
    EXPECT_FALSE(isRealModifier(SC_SPACE));
}

//==============================================================================
// isVirtualModifier tests
//==============================================================================

TEST(Modifiers, IsVirtualModifier_VirtualModifiers) {
    EXPECT_TRUE(isVirtualModifier(VK_MOD9));
    EXPECT_TRUE(isVirtualModifier(VK_MOD10));
    EXPECT_TRUE(isVirtualModifier(VK_MOD11));
    EXPECT_TRUE(isVirtualModifier(VK_MOD12));
    EXPECT_TRUE(isVirtualModifier(VK_MOD13));
    EXPECT_TRUE(isVirtualModifier(VK_MOD14));
    EXPECT_TRUE(isVirtualModifier(VK_MOD15));
}

TEST(Modifiers, IsVirtualModifier_RealModifiersAreFalse) {
    EXPECT_FALSE(isVirtualModifier(SC_LSHIFT));
    EXPECT_FALSE(isVirtualModifier(SC_RSHIFT));
    EXPECT_FALSE(isVirtualModifier(SC_LCTRL));
    EXPECT_FALSE(isVirtualModifier(SC_RCTRL));
}

TEST(Modifiers, IsVirtualModifier_NonModifiers) {
    EXPECT_FALSE(isVirtualModifier(SC_A));
    EXPECT_FALSE(isVirtualModifier(SC_SPACE));
}

//==============================================================================
// Bitmask roundtrip tests (vcode -> bitmask -> vcode)
//==============================================================================

TEST(Modifiers, BitmaskRoundtrip_RealModifiers) {
    // Test that we can convert to bitmask and back
    EXPECT_EQ(getModifierForBitmask(getModifierBitmaskForVcode(SC_LSHIFT)), SC_LSHIFT);
    EXPECT_EQ(getModifierForBitmask(getModifierBitmaskForVcode(SC_RSHIFT)), SC_RSHIFT);
    EXPECT_EQ(getModifierForBitmask(getModifierBitmaskForVcode(SC_LCTRL)), SC_LCTRL);
    EXPECT_EQ(getModifierForBitmask(getModifierBitmaskForVcode(SC_LALT)), SC_LALT);
}

TEST(Modifiers, BitmaskRoundtrip_VirtualModifiers) {
    EXPECT_EQ(getModifierForBitmask(getModifierBitmaskForVcode(VK_MOD9)), VK_MOD9);
    EXPECT_EQ(getModifierForBitmask(getModifierBitmaskForVcode(VK_MOD10)), VK_MOD10);
    EXPECT_EQ(getModifierForBitmask(getModifierBitmaskForVcode(VK_MOD15)), VK_MOD15);
}
