/**
 * test_key_processor.cpp
 * 
 * Unit tests for the KeyProcessor orchestration component.
 * Tests the full key processing pipeline.
 */

#include <gtest/gtest.h>
#include "domain/KeyProcessor.h"

using namespace capsicain::domain;

// Test constants
constexpr uint16_t KEY_A = 0x1E;
constexpr uint16_t KEY_B = 0x30;
constexpr uint16_t KEY_C = 0x2E;
constexpr uint16_t KEY_ESC = 0x01;
constexpr uint16_t KEY_TAB = 0x0F;
constexpr uint16_t KEY_LSHIFT = 0x2A;
constexpr uint16_t KEY_LCTRL = 0x1D;
constexpr uint16_t KEY_NOP = 0xFF;

// Mock modifier info for testing
class MockModifierInfo : public IModifierInfo {
public:
    MOD modifiersDown = 0;
    MOD modifiersTapped = 0;
    uint8_t deadkey = 0;
    
    bool isModifier(VKeyCode vcode) const override {
        return vcode == KEY_LSHIFT || vcode == KEY_LCTRL;
    }
    
    MOD getModifierBitmask(VKeyCode vcode) const override {
        if (vcode == KEY_LSHIFT) return 0x0001;
        if (vcode == KEY_LCTRL) return 0x0004;
        return 0;
    }
    
    MOD getModifiersDown() const override { return modifiersDown; }
    MOD getModifiersTapped() const override { return modifiersTapped; }
    uint8_t getActiveDeadkey() const override { return deadkey; }
};

// Helper to create empty combos
std::vector<ComboRule> emptyCombos() { return {}; }

// Helper for rewire that returns no mapping
RewireEntry noRewire(uint16_t) { return RewireEntry{}; }

// Identity alpha map
int identityAlphaMap[MAX_VCODES];

class KeyProcessorTest : public ::testing::Test {
protected:
    KeyProcessor processor;
    KeyProcessorConfig config;
    MockModifierInfo modInfo;
    
    void SetUp() override {
        config = KeyProcessorConfig{};
        modInfo = MockModifierInfo{};
        
        // Initialize identity alpha map
        for (size_t i = 0; i < MAX_VCODES; ++i) {
            identityAlphaMap[i] = static_cast<int>(i);
        }
    }
};


// ============================================================================
// Basic Passthrough Tests
// ============================================================================

TEST_F(KeyProcessorTest, PassthroughKey_NoMappings) {
    KeyProcessorInput input;
    input.scancode = KEY_A;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, noRewire, identityAlphaMap,
        emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_EQ(output.finalVcode, KEY_A);
    ASSERT_EQ(output.events.size(), 1u);
    EXPECT_EQ(output.events[0].vcode, KEY_A);
    EXPECT_TRUE(output.events[0].isDown);
    EXPECT_FALSE(output.consumed);
}

TEST_F(KeyProcessorTest, PassthroughKey_KeyUp) {
    KeyProcessorInput input;
    input.scancode = KEY_A;
    input.isDown = false;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, noRewire, identityAlphaMap,
        emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_EQ(output.finalVcode, KEY_A);
    ASSERT_EQ(output.events.size(), 1u);
    EXPECT_FALSE(output.events[0].isDown);
}


// ============================================================================
// Alpha Mapping Tests
// ============================================================================

TEST_F(KeyProcessorTest, AlphaMapping_RemapsKey) {
    int alphaMap[MAX_VCODES];
    for (size_t i = 0; i < MAX_VCODES; ++i) alphaMap[i] = static_cast<int>(i);
    alphaMap[KEY_A] = KEY_B;  // Remap A to B
    
    KeyProcessorInput input;
    input.scancode = KEY_A;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, noRewire, alphaMap,
        emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_EQ(output.finalVcode, KEY_B);
}

TEST_F(KeyProcessorTest, AlphaMapping_FlipZY) {
    config.flipZY = true;
    constexpr uint16_t KEY_Y = 0x15;
    constexpr uint16_t KEY_Z = 0x2C;
    
    KeyProcessorInput input;
    input.scancode = KEY_Z;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, noRewire, identityAlphaMap,
        emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_EQ(output.finalVcode, KEY_Y);
}

TEST_F(KeyProcessorTest, AlphaMapping_Disabled) {
    int alphaMap[MAX_VCODES];
    for (size_t i = 0; i < MAX_VCODES; ++i) alphaMap[i] = static_cast<int>(i);
    alphaMap[KEY_A] = KEY_B;
    
    config.processAlphaMapping = false;
    
    KeyProcessorInput input;
    input.scancode = KEY_A;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, noRewire, alphaMap,
        emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_EQ(output.finalVcode, KEY_A);  // No mapping applied
}


// ============================================================================
// Rewire Mapping Tests
// ============================================================================

TEST_F(KeyProcessorTest, Rewire_SimpleRemap) {
    auto getRewire = [](uint16_t sc) -> RewireEntry {
        if (sc == KEY_TAB) {
            RewireEntry entry;
            entry.outKey = KEY_ESC;
            return entry;
        }
        return RewireEntry{};
    };
    
    KeyProcessorInput input;
    input.scancode = KEY_TAB;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, getRewire, identityAlphaMap,
        emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_EQ(output.finalVcode, KEY_ESC);
}

TEST_F(KeyProcessorTest, Rewire_ToModifier_SetsFlag) {
    auto getRewire = [](uint16_t sc) -> RewireEntry {
        if (sc == KEY_TAB) {
            RewireEntry entry;
            entry.outKey = KEY_LSHIFT;
            return entry;
        }
        return RewireEntry{};
    };
    
    KeyProcessorInput input;
    input.scancode = KEY_TAB;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, getRewire, identityAlphaMap,
        emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_TRUE(output.isModifier);
}

TEST_F(KeyProcessorTest, Rewire_Disabled) {
    auto getRewire = [](uint16_t sc) -> RewireEntry {
        if (sc == KEY_TAB) {
            RewireEntry entry;
            entry.outKey = KEY_ESC;
            return entry;
        }
        return RewireEntry{};
    };
    
    config.processRewiring = false;
    
    KeyProcessorInput input;
    input.scancode = KEY_TAB;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, getRewire, identityAlphaMap,
        emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_EQ(output.finalVcode, KEY_TAB);  // No rewire applied
}


// ============================================================================
// Combo Matching Tests
// ============================================================================

TEST_F(KeyProcessorTest, Combo_MatchesDownstroke) {
    ComboRule rule;
    rule.triggerKey = KEY_A;
    rule.resultSequence = {{KEY_B, true}, {KEY_B, false}};
    
    std::vector<ComboRule> downCombos = {rule};
    
    KeyProcessorInput input;
    input.scancode = KEY_A;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, noRewire, identityAlphaMap,
        downCombos, emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_TRUE(output.consumed);
    ASSERT_EQ(output.events.size(), 2u);
    EXPECT_EQ(output.events[0].vcode, KEY_B);
    EXPECT_TRUE(output.events[0].isDown);
    EXPECT_EQ(output.events[1].vcode, KEY_B);
    EXPECT_FALSE(output.events[1].isDown);
}

TEST_F(KeyProcessorTest, Combo_WithModifierRequirement) {
    ComboRule rule;
    rule.triggerKey = KEY_A;
    rule.modAnd = 0x0001;  // Requires LShift
    rule.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> downCombos = {rule};
    
    KeyProcessorInput input;
    input.scancode = KEY_A;
    input.isDown = true;
    
    // Without modifier - no match
    modInfo.modifiersDown = 0;
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, noRewire, identityAlphaMap,
        downCombos, emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_FALSE(output.consumed);
    EXPECT_EQ(output.finalVcode, KEY_A);
    
    // With modifier - match
    modInfo.modifiersDown = 0x0001;
    output = processor.process(
        input, config, &modInfo, noRewire, identityAlphaMap,
        downCombos, emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_TRUE(output.consumed);
    EXPECT_EQ(output.events[0].vcode, KEY_C);
}

TEST_F(KeyProcessorTest, Combo_Disabled) {
    ComboRule rule;
    rule.triggerKey = KEY_A;
    rule.resultSequence = {{KEY_B, true}};
    
    std::vector<ComboRule> downCombos = {rule};
    
    config.processCombos = false;
    
    KeyProcessorInput input;
    input.scancode = KEY_A;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, noRewire, identityAlphaMap,
        downCombos, emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_FALSE(output.consumed);
    EXPECT_EQ(output.finalVcode, KEY_A);  // Combo not matched
}


// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(KeyProcessorTest, Integration_RewireThenAlphaMap) {
    // Rewire TAB -> A, then alpha map A -> B
    auto getRewire = [](uint16_t sc) -> RewireEntry {
        if (sc == KEY_TAB) {
            RewireEntry entry;
            entry.outKey = KEY_A;
            return entry;
        }
        return RewireEntry{};
    };
    
    int alphaMap[MAX_VCODES];
    for (size_t i = 0; i < MAX_VCODES; ++i) alphaMap[i] = static_cast<int>(i);
    alphaMap[KEY_A] = KEY_B;
    
    KeyProcessorInput input;
    input.scancode = KEY_TAB;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, getRewire, alphaMap,
        emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_EQ(output.finalVcode, KEY_B);  // TAB -> A -> B
}

TEST_F(KeyProcessorTest, Integration_ComboTakesPriority) {
    // Both rewire and combo defined - combo should win
    auto getRewire = [](uint16_t sc) -> RewireEntry {
        if (sc == KEY_A) {
            RewireEntry entry;
            entry.outKey = KEY_B;
            return entry;
        }
        return RewireEntry{};
    };
    
    ComboRule rule;
    rule.triggerKey = KEY_B;  // Matches after rewire
    rule.resultSequence = {{KEY_C, true}};
    
    std::vector<ComboRule> downCombos = {rule};
    
    KeyProcessorInput input;
    input.scancode = KEY_A;
    input.isDown = true;
    
    KeyProcessorOutput output = processor.process(
        input, config, &modInfo, getRewire, identityAlphaMap,
        downCombos, emptyCombos(), emptyCombos(), emptyCombos(), emptyCombos()
    );
    
    EXPECT_TRUE(output.consumed);
    EXPECT_EQ(output.events[0].vcode, KEY_C);  // Combo result
}
