/**
 * @file test_tap_detector.cpp
 * @brief Unit tests for TapDetector
 */

#include <gtest/gtest.h>
#include "domain/TapDetector.h"

using namespace capsicain;

class TapDetectorTest : public ::testing::Test {
protected:
    TapDetector detector;
    
    // Helper to create key events
    KeyEvent down(int vcode) { return KeyEvent(vcode, true); }
    KeyEvent up(int vcode) { return KeyEvent(vcode, false); }
};

//==============================================================================
// Basic tap detection
//==============================================================================

TEST_F(TapDetectorTest, DetectsTap_SingleKeyDownThenUp) {
    // Sequence: A down -> A up
    KeyEvent prev2 = down(0x00);  // Nothing (NOP)
    KeyEvent prev1 = down(0x1E);  // A down
    KeyEvent current = up(0x1E); // A up
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_TRUE(result.tapped);
    EXPECT_FALSE(result.tappedSlow);
    EXPECT_FALSE(result.tapHoldMake);
    EXPECT_FALSE(result.repeat);
}

TEST_F(TapDetectorTest, NoTap_WhenKeyIsDown) {
    // Current is down, not a tap
    KeyEvent prev2 = down(0x00);
    KeyEvent prev1 = down(0x1E);  // A down
    KeyEvent current = down(0x1E); // A down (different scenario)
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_FALSE(result.tapped);
}

TEST_F(TapDetectorTest, NoTap_WhenDifferentKeys) {
    // A down -> B up = not a tap
    KeyEvent prev2 = down(0x00);
    KeyEvent prev1 = down(0x1E);  // A down
    KeyEvent current = up(0x30); // B up
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_FALSE(result.tapped);
}

//==============================================================================
// Slow tap detection
//==============================================================================

TEST_F(TapDetectorTest, DetectsSlowTap_WithAutoRepeat) {
    // Sequence: A down -> A down (repeat) -> A up
    KeyEvent prev2 = down(0x1E);  // A down (original)
    KeyEvent prev1 = down(0x1E);  // A down (repeat)
    KeyEvent current = up(0x1E); // A up
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_FALSE(result.tapped);      // Slow tap, not regular tap
    EXPECT_TRUE(result.tappedSlow);
    EXPECT_FALSE(result.tapHoldMake);
}

TEST_F(TapDetectorTest, SlowTap_OverridesRegularTap) {
    // When slow tap is detected, tapped should be false
    KeyEvent prev2 = down(0x1E);
    KeyEvent prev1 = down(0x1E);
    KeyEvent current = up(0x1E);
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_FALSE(result.tapped);
    EXPECT_TRUE(result.tappedSlow);
}

//==============================================================================
// Tap-hold detection
//==============================================================================

TEST_F(TapDetectorTest, DetectsTapHold_DownUpDownSequence) {
    // Sequence: A down -> A up -> A down
    KeyEvent prev2 = down(0x1E);  // A down
    KeyEvent prev1 = up(0x1E);    // A up
    KeyEvent current = down(0x1E); // A down again
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_FALSE(result.tapped);
    EXPECT_FALSE(result.tappedSlow);
    EXPECT_TRUE(result.tapHoldMake);
}

TEST_F(TapDetectorTest, NoTapHold_WhenDifferentKeys) {
    // A down -> A up -> B down = not tap-hold
    KeyEvent prev2 = down(0x1E);  // A down
    KeyEvent prev1 = up(0x1E);    // A up
    KeyEvent current = down(0x30); // B down
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_FALSE(result.tapHoldMake);
}

TEST_F(TapDetectorTest, NoTapHold_WhenCurrentIsUp) {
    // Tap-hold only triggers on the second down
    KeyEvent prev2 = down(0x1E);
    KeyEvent prev1 = up(0x1E);
    KeyEvent current = up(0x1E);
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_FALSE(result.tapHoldMake);
}

//==============================================================================
// Repeat detection
//==============================================================================

TEST_F(TapDetectorTest, DetectsRepeat_ConsecutiveDowns) {
    // Sequence: A down -> A down (auto-repeat)
    KeyEvent prev2 = down(0x00);
    KeyEvent prev1 = down(0x1E);  // A down
    KeyEvent current = down(0x1E); // A down (repeat)
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_TRUE(result.repeat);
}

TEST_F(TapDetectorTest, NoRepeat_WhenDifferentKeys) {
    // A down -> B down = not a repeat
    KeyEvent prev2 = down(0x00);
    KeyEvent prev1 = down(0x1E);  // A down
    KeyEvent current = down(0x30); // B down
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_FALSE(result.repeat);
}

TEST_F(TapDetectorTest, NoRepeat_WhenKeyUp) {
    // A down -> A up = not a repeat
    KeyEvent prev2 = down(0x00);
    KeyEvent prev1 = down(0x1E);
    KeyEvent current = up(0x1E);
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_FALSE(result.repeat);
}

//==============================================================================
// Edge cases
//==============================================================================

TEST_F(TapDetectorTest, HandlesNopKey) {
    // NOP (0) key sequence - but prev1 needs to be the same key
    // This tests that vcode 0 works correctly
    KeyEvent prev2 = down(0x1E);  // Some other key
    KeyEvent prev1 = down(0x00);  // NOP down
    KeyEvent current = up(0x00); // NOP up
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    // Should detect tap for NOP (vcode 0) - prev2 is different so not slow tap
    EXPECT_TRUE(result.tapped);
}

TEST_F(TapDetectorTest, ModifierKeyTap) {
    // LSHIFT tap
    const int SC_LSHIFT = 0x2A;
    KeyEvent prev2 = down(0x00);
    KeyEvent prev1 = down(SC_LSHIFT);
    KeyEvent current = up(SC_LSHIFT);
    
    TapResult result = detector.detect(current, prev1, prev2);
    
    EXPECT_TRUE(result.tapped);
}

TEST_F(TapDetectorTest, TapResultReset) {
    TapResult result;
    result.tapped = true;
    result.tappedSlow = true;
    result.tapHoldMake = true;
    result.repeat = true;
    
    result.reset();
    
    EXPECT_FALSE(result.tapped);
    EXPECT_FALSE(result.tappedSlow);
    EXPECT_FALSE(result.tapHoldMake);
    EXPECT_FALSE(result.repeat);
}
