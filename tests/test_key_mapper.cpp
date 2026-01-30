/**
 * test_key_mapper.cpp
 * 
 * Unit tests for the KeyMapper domain component.
 * Tests alpha key mapping and rewire mapping logic.
 */

#include <gtest/gtest.h>
#include "domain/KeyMapper.h"

using namespace capsicain::domain;

// Test constants
constexpr VKeyCode KEY_A = 0x1E;
constexpr VKeyCode KEY_B = 0x30;
constexpr VKeyCode KEY_C = 0x2E;
constexpr VKeyCode KEY_Y = 0x15;
constexpr VKeyCode KEY_Z = 0x2C;
constexpr VKeyCode KEY_TAB = 0x0F;
constexpr VKeyCode KEY_ESC = 0x01;
constexpr VKeyCode KEY_LSHIFT = 0x2A;
constexpr VKeyCode KEY_LCTRL = 0x1D;

// Mock modifier query for testing
class MockModifierQuery : public IModifierQuery {
public:
    bool isModifier(VKeyCode vcode) const override {
        return vcode == KEY_LSHIFT || vcode == KEY_LCTRL;
    }
    
    MOD getModifierBitmask(VKeyCode vcode) const override {
        if (vcode == KEY_LSHIFT) return 0x0001;
        if (vcode == KEY_LCTRL) return 0x0004;
        return 0;
    }
};


// ============================================================================
// Alpha Mapping Tests
// ============================================================================

class AlphaMappingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize identity mapping
        for (size_t i = 0; i < MAX_VCODES; ++i) {
            alphaMap[i] = static_cast<int>(i);
        }
        options = AlphaMapOptions{};
    }
    
    int alphaMap[MAX_VCODES];
    AlphaMapOptions options;
};

TEST_F(AlphaMappingTest, IdentityMapping_NoChange) {
    AlphaMapResult result = applyAlphaMapping(KEY_A, alphaMap, options, false, false, false);
    EXPECT_EQ(result.mappedKey, KEY_A);
    EXPECT_FALSE(result.wasRemapped);
}

TEST_F(AlphaMappingTest, RemapsKey) {
    alphaMap[KEY_A] = KEY_B;  // Remap A to B
    
    AlphaMapResult result = applyAlphaMapping(KEY_A, alphaMap, options, false, false, false);
    EXPECT_EQ(result.mappedKey, KEY_B);
    EXPECT_TRUE(result.wasRemapped);
}

TEST_F(AlphaMappingTest, ModifierKey_NoMapping) {
    alphaMap[KEY_LSHIFT] = KEY_A;  // Try to remap shift
    
    AlphaMapResult result = applyAlphaMapping(KEY_LSHIFT, alphaMap, options, true, false, false);
    EXPECT_EQ(result.mappedKey, KEY_LSHIFT);  // Unchanged
    EXPECT_FALSE(result.wasRemapped);
}

TEST_F(AlphaMappingTest, CtrlWinBlocksMapping_WithCtrl) {
    alphaMap[KEY_A] = KEY_B;
    options.ctrlWinBlocksAlphaMapping = true;
    
    AlphaMapResult result = applyAlphaMapping(KEY_A, alphaMap, options, false, true, false);
    EXPECT_EQ(result.mappedKey, KEY_A);  // Unchanged due to Ctrl
    EXPECT_FALSE(result.wasRemapped);
}

TEST_F(AlphaMappingTest, CtrlWinBlocksMapping_WithWin) {
    alphaMap[KEY_A] = KEY_B;
    options.ctrlWinBlocksAlphaMapping = true;
    
    AlphaMapResult result = applyAlphaMapping(KEY_A, alphaMap, options, false, false, true);
    EXPECT_EQ(result.mappedKey, KEY_A);  // Unchanged due to Win
    EXPECT_FALSE(result.wasRemapped);
}

TEST_F(AlphaMappingTest, CtrlWinBlocksMapping_OptionOff) {
    alphaMap[KEY_A] = KEY_B;
    options.ctrlWinBlocksAlphaMapping = false;
    
    AlphaMapResult result = applyAlphaMapping(KEY_A, alphaMap, options, false, true, true);
    EXPECT_EQ(result.mappedKey, KEY_B);  // Still remaps
    EXPECT_TRUE(result.wasRemapped);
}

TEST_F(AlphaMappingTest, FlipZY_ZtoY) {
    options.flipZY = true;
    
    AlphaMapResult result = applyAlphaMapping(KEY_Z, alphaMap, options, false, false, false);
    EXPECT_EQ(result.mappedKey, KEY_Y);
    EXPECT_TRUE(result.wasRemapped);
}

TEST_F(AlphaMappingTest, FlipZY_YtoZ) {
    options.flipZY = true;
    
    AlphaMapResult result = applyAlphaMapping(KEY_Y, alphaMap, options, false, false, false);
    EXPECT_EQ(result.mappedKey, KEY_Z);
    EXPECT_TRUE(result.wasRemapped);
}

TEST_F(AlphaMappingTest, FlipZY_OtherKeys_NoChange) {
    options.flipZY = true;
    
    AlphaMapResult result = applyAlphaMapping(KEY_A, alphaMap, options, false, false, false);
    EXPECT_EQ(result.mappedKey, KEY_A);
    EXPECT_FALSE(result.wasRemapped);
}

TEST_F(AlphaMappingTest, FlipZY_AfterAlphaMapping) {
    // First remap A to Z, then flip Z to Y
    alphaMap[KEY_A] = KEY_Z;
    options.flipZY = true;
    
    AlphaMapResult result = applyAlphaMapping(KEY_A, alphaMap, options, false, false, false);
    EXPECT_EQ(result.mappedKey, KEY_Y);  // A -> Z -> Y
    EXPECT_TRUE(result.wasRemapped);
}

TEST_F(AlphaMappingTest, NullAlphaMap_ReturnsInput) {
    AlphaMapResult result = applyAlphaMapping(KEY_A, nullptr, options, false, false, false);
    EXPECT_EQ(result.mappedKey, KEY_A);
    EXPECT_FALSE(result.wasRemapped);
}


// ============================================================================
// Rewire Mapping Tests
// ============================================================================

class RewireMappingTest : public ::testing::Test {
protected:
    RewireContext context;
    RewireEntry entry;
    MockModifierQuery modQuery;
    
    void SetUp() override {
        context = RewireContext{};
        context.scancode = KEY_TAB;
        context.vcode = KEY_TAB;
        context.isDownstroke = true;
        
        entry = RewireEntry{};
    }
};

TEST_F(RewireMappingTest, NoRewire_PassThrough) {
    // No rewire defined
    RewireResult result = applyRewireMapping(context, entry);
    EXPECT_EQ(result.outputKey, KEY_TAB);
    EXPECT_TRUE(result.eventSequence.empty());
    EXPECT_FALSE(result.shouldNop);
}

TEST_F(RewireMappingTest, SimpleRewire) {
    entry.outKey = KEY_ESC;  // Tab -> Esc
    
    RewireResult result = applyRewireMapping(context, entry);
    EXPECT_EQ(result.outputKey, KEY_ESC);
    EXPECT_TRUE(result.eventSequence.empty());  // No extra events
}

TEST_F(RewireMappingTest, TapAndHold_SuppressesRepeat) {
    context.activeTapHoldKey = KEY_TAB;  // Tab is in tap-hold state
    entry.outKey = KEY_LSHIFT;
    
    RewireResult result = applyRewireMapping(context, entry);
    EXPECT_EQ(result.outputKey, SC_NOP);
    EXPECT_TRUE(result.shouldNop);
}

TEST_F(RewireMappingTest, TapRewire_SendsSequence) {
    context.isTapped = true;
    entry.outKey = KEY_LSHIFT;
    entry.tapKey = KEY_ESC;  // Tap -> Esc
    
    RewireResult result = applyRewireMapping(context, entry, &modQuery);
    
    EXPECT_EQ(result.outputKey, KEY_ESC);
    EXPECT_EQ(result.tappedToClear, 0xFFFFFFFFu);  // All cleared
    
    // Should have: Shift up, Esc down, Esc up
    ASSERT_EQ(result.eventSequence.size(), 3u);
    EXPECT_EQ(result.eventSequence[0].keyCode, KEY_LSHIFT);
    EXPECT_FALSE(result.eventSequence[0].isDown);  // Release shift
    EXPECT_EQ(result.eventSequence[1].keyCode, KEY_ESC);
    EXPECT_TRUE(result.eventSequence[1].isDown);   // Esc down
    EXPECT_EQ(result.eventSequence[2].keyCode, KEY_ESC);
    EXPECT_FALSE(result.eventSequence[2].isDown);  // Esc up
}

TEST_F(RewireMappingTest, TapRewire_ClearsModifierDown) {
    context.isTapped = true;
    entry.outKey = KEY_LSHIFT;
    entry.tapKey = KEY_ESC;
    
    RewireResult result = applyRewireMapping(context, entry, &modQuery);
    
    // Should clear LShift bitmask
    EXPECT_EQ(result.modifiersToClear, 0x0001u);
}

TEST_F(RewireMappingTest, TapHoldMake_FirstActivation) {
    context.isTapHoldMake = true;
    context.activeTapHoldKey = 0;  // No active tap-hold
    entry.outKey = KEY_LSHIFT;
    entry.tapHoldKey = KEY_LCTRL;  // Tap-hold -> Ctrl
    
    RewireResult result = applyRewireMapping(context, entry, &modQuery);
    
    EXPECT_EQ(result.outputKey, KEY_LCTRL);
    EXPECT_EQ(result.newTapHoldKey, KEY_TAB);  // Remember the scancode
    
    // Should send Ctrl down
    ASSERT_EQ(result.eventSequence.size(), 1u);
    EXPECT_EQ(result.eventSequence[0].keyCode, KEY_LCTRL);
    EXPECT_TRUE(result.eventSequence[0].isDown);
}

TEST_F(RewireMappingTest, TapHoldMake_SecondIgnored) {
    context.isTapHoldMake = true;
    context.activeTapHoldKey = KEY_A;  // Already have an active tap-hold
    entry.outKey = KEY_LSHIFT;
    entry.tapHoldKey = KEY_LCTRL;
    
    RewireResult result = applyRewireMapping(context, entry, &modQuery);
    
    // Should not activate new tap-hold
    EXPECT_EQ(result.newTapHoldKey, 0u);  // No change
    EXPECT_TRUE(result.eventSequence.empty());
}

TEST_F(RewireMappingTest, TapHoldBreak_ReleasesKey) {
    context.isDownstroke = false;
    context.activeTapHoldKey = KEY_TAB;
    entry.outKey = KEY_LSHIFT;
    entry.tapHoldKey = KEY_LCTRL;
    
    RewireResult result = applyRewireMapping(context, entry, &modQuery);
    
    EXPECT_EQ(result.outputKey, KEY_LCTRL);
    EXPECT_EQ(result.newTapHoldKey, static_cast<ScanCode>(-1));  // Signal to clear
    
    // Should send Ctrl up
    ASSERT_EQ(result.eventSequence.size(), 1u);
    EXPECT_EQ(result.eventSequence[0].keyCode, KEY_LCTRL);
    EXPECT_FALSE(result.eventSequence[0].isDown);
}

TEST_F(RewireMappingTest, TapHoldBreak_VirtualKey_Nops) {
    context.isDownstroke = false;
    context.activeTapHoldKey = KEY_TAB;
    entry.outKey = KEY_LSHIFT;
    entry.tapHoldKey = 300;  // Virtual key > 255
    
    RewireResult result = applyRewireMapping(context, entry, &modQuery);
    
    EXPECT_TRUE(result.shouldNop);
    EXPECT_TRUE(result.eventSequence.empty());  // No break sent for virtual keys
}

TEST_F(RewireMappingTest, OutputIsModifier_SetFlag) {
    entry.outKey = KEY_LSHIFT;
    
    RewireResult result = applyRewireMapping(context, entry, &modQuery);
    
    EXPECT_TRUE(result.isModifier);
}

TEST_F(RewireMappingTest, OutputIsNotModifier_ClearFlag) {
    entry.outKey = KEY_A;
    
    RewireResult result = applyRewireMapping(context, entry, &modQuery);
    
    EXPECT_FALSE(result.isModifier);
}


// ============================================================================
// KeyMapper Class Tests
// ============================================================================

TEST(KeyMapperTest, MapAlpha_DelegatesToPureFunction) {
    KeyMapper mapper;
    int alphaMap[MAX_VCODES];
    for (size_t i = 0; i < MAX_VCODES; ++i) alphaMap[i] = static_cast<int>(i);
    alphaMap[KEY_A] = KEY_B;
    
    AlphaMapOptions options;
    AlphaMapResult result = mapper.mapAlpha(KEY_A, alphaMap, options, false, false, false);
    
    EXPECT_EQ(result.mappedKey, KEY_B);
    EXPECT_TRUE(result.wasRemapped);
}

TEST(KeyMapperTest, MapRewire_DelegatesToPureFunction) {
    KeyMapper mapper;
    MockModifierQuery modQuery;
    
    RewireContext context;
    context.scancode = KEY_TAB;
    context.vcode = KEY_TAB;
    context.isDownstroke = true;
    
    RewireEntry entry;
    entry.outKey = KEY_ESC;
    
    RewireResult result = mapper.mapRewire(context, entry, &modQuery);
    
    EXPECT_EQ(result.outputKey, KEY_ESC);
}
