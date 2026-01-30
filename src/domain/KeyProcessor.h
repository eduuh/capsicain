#pragma once
/**
 * KeyProcessor.h
 * 
 * Main processing pipeline that composes all domain components.
 * This is the central orchestrator for key transformation logic.
 * 
 * Processing Pipeline:
 * 1. Receive raw key event
 * 2. Detect tap/hold patterns (TapDetector)
 * 3. Apply rewire mapping (KeyMapper)
 * 4. Update modifier state (ModifierTracker)
 * 5. Match combos (ComboMatcher)
 * 6. Apply alpha mapping (KeyMapper)
 * 7. Emit result key events
 */

#include <vector>
#include <functional>
#include <cstdint>
#include "TapDetector.h"
#include "ModifierTracker.h"
#include "ComboMatcher.h"
#include "KeyMapper.h"

namespace capsicain {
namespace domain {

/**
 * Simple key event for output
 */
struct ProcessedKeyEvent {
    uint16_t vcode = 0;
    bool isDown = true;
};

/**
 * Configuration for the key processor
 */
struct KeyProcessorConfig {
    // Alpha mapping options
    bool flipZY = false;
    bool ctrlWinBlocksAlphaMapping = false;
    
    // Processing options
    bool processAlphaMapping = true;
    bool processRewiring = true;
    bool processCombos = true;
};

/**
 * Input to the key processor
 */
struct KeyProcessorInput {
    uint16_t scancode = 0;          // Hardware scancode
    bool isDown = true;             // Key direction
    uint16_t deviceMask = 0;        // Device identifier mask
    
    // Previous keystrokes for tap detection
    uint16_t prev1Scancode = 0;
    bool prev1IsDown = false;
    uint16_t prev2Scancode = 0;
    bool prev2IsDown = false;
};

/**
 * Output from the key processor
 */
struct KeyProcessorOutput {
    std::vector<ProcessedKeyEvent> events;  // Key events to send
    bool consumed = false;                   // Whether input was consumed (no passthrough)
    bool isModifier = false;                 // Whether result is a modifier key
    uint16_t finalVcode = 0;                 // Final virtual keycode
};

/**
 * Callback interface for modifier queries (bridges to existing modifiers.cpp)
 */
class IModifierInfo : public IModifierQuery {
public:
    virtual MOD getModifiersDown() const = 0;
    virtual MOD getModifiersTapped() const = 0;
    virtual uint8_t getActiveDeadkey() const = 0;
};

/**
 * KeyProcessor - Main key transformation pipeline
 * 
 * This class composes all domain components into a single processing unit.
 * It is stateless - all state is passed in via context objects.
 */
class KeyProcessor {
public:
    /**
     * Process a single key event through the full pipeline
     */
    KeyProcessorOutput process(
        const KeyProcessorInput& input,
        const KeyProcessorConfig& config,
        const IModifierInfo* modInfo,
        const std::function<RewireEntry(uint16_t)>& getRewireEntry,
        const int* alphaMap,
        const std::vector<ComboRule>& downCombos,
        const std::vector<ComboRule>& upCombos,
        const std::vector<ComboRule>& tapCombos,
        const std::vector<ComboRule>& slowCombos,
        const std::vector<ComboRule>& repeatCombos
    ) const {
        KeyProcessorOutput output;
        
        // Step 1: Detect tap patterns using capsicain::TapDetector
        capsicain::KeyEvent current{static_cast<int>(input.scancode), input.isDown};
        capsicain::KeyEvent prev1{static_cast<int>(input.prev1Scancode), input.prev1IsDown};
        capsicain::KeyEvent prev2{static_cast<int>(input.prev2Scancode), input.prev2IsDown};
        
        capsicain::TapResult tapResult = m_tapDetector.detect(current, prev1, prev2);
        
        // Step 2: Apply rewire mapping
        uint16_t vcode = input.scancode;
        RewireEntry rewireEntry = getRewireEntry(input.scancode);
        
        if (config.processRewiring && rewireEntry.hasOutput()) {
            RewireContext rewireCtx;
            rewireCtx.scancode = input.scancode;
            rewireCtx.vcode = vcode;
            rewireCtx.isDownstroke = input.isDown;
            rewireCtx.isTapped = tapResult.tapped;
            rewireCtx.isTapHoldMake = tapResult.tapHoldMake;
            
            RewireResult rewireResult = m_keyMapper.mapRewire(rewireCtx, rewireEntry, modInfo);
            
            vcode = rewireResult.outputKey;
            output.isModifier = rewireResult.isModifier;
            
            // Add any generated events
            for (const auto& evt : rewireResult.eventSequence) {
                output.events.push_back({evt.keyCode, evt.isDown});
            }
            
            if (rewireResult.shouldNop) {
                output.consumed = true;
                return output;
            }
        }
        
        // Step 3: Process combos
        if (config.processCombos && modInfo != nullptr) {
            ComboMatchContext comboCtx;
            comboCtx.currentKey = vcode;
            comboCtx.modifiersDown = modInfo->getModifiersDown();
            comboCtx.modifiersTapped = modInfo->getModifiersTapped();
            comboCtx.deviceMask = input.deviceMask;
            comboCtx.activeDeadkey = modInfo->getActiveDeadkey();
            
            ComboMatchResult comboResult;
            
            if (input.isDown) {
                comboResult = m_comboMatcher.matchDownstroke(
                    downCombos, repeatCombos, comboCtx, tapResult.repeat);
            } else {
                comboResult = m_comboMatcher.matchUpstroke(
                    upCombos, tapCombos, slowCombos, comboCtx,
                    tapResult.tapped, tapResult.tappedSlow);
            }
            
            if (comboResult.matched) {
                // Replace output with combo result
                output.events.clear();
                for (const auto& evt : comboResult.resultSequence) {
                    output.events.push_back({evt.keyCode, evt.isDown});
                }
                output.consumed = true;
                output.finalVcode = output.events.empty() ? vcode : output.events.back().vcode;
                return output;
            }
        }
        
        // Step 4: Apply alpha mapping
        if (config.processAlphaMapping && alphaMap != nullptr && !output.isModifier) {
            AlphaMapOptions alphaOpts;
            alphaOpts.flipZY = config.flipZY;
            alphaOpts.ctrlWinBlocksAlphaMapping = config.ctrlWinBlocksAlphaMapping;
            
            bool isCtrlDown = modInfo ? (modInfo->getModifiersDown() & 0x000C) != 0 : false;
            bool isWinDown = modInfo ? (modInfo->getModifiersDown() & 0x00C0) != 0 : false;
            
            AlphaMapResult alphaResult = m_keyMapper.mapAlpha(
                vcode, alphaMap, alphaOpts, output.isModifier, isCtrlDown, isWinDown);
            
            vcode = alphaResult.mappedKey;
        }
        
        // Step 5: Generate final output
        output.finalVcode = vcode;
        if (output.events.empty()) {
            output.events.push_back({vcode, input.isDown});
        }
        
        return output;
    }
    
private:
    capsicain::TapDetector m_tapDetector;
    ComboMatcher m_comboMatcher;
    KeyMapper m_keyMapper;
};

} // namespace domain
} // namespace capsicain
