#include "services/KeyProcessingService.h"
#include "services/ConfigurationService.h"
#include "services/RuntimeStateService.h"
#include "services/HardwareService.h"
#include "services/UIService.h"
#include "legacy/scancodes.h"
#include "legacy/modifiers.h"

namespace capsicain {
namespace services {

KeyProcessingService::KeyProcessingService(
    ConfigurationService& config,
    RuntimeStateService& runtime,
    HardwareService& hardware,
    UIService& ui
)
    : config_(config)
    , runtime_(runtime)
    , hardware_(hardware)
    , ui_(ui)
    , keyMapper_()
    , comboMatcher_()
{
}

ProcessResult KeyProcessingService::processKeystroke(const HardwareKeystroke& keystroke)
{
    ProcessResult result;
    result.shouldForward = false;
    result.consumed = false;

    // Stage 1: Reset context
    stage1_ResetContext();

    // Initialize context from keystroke
    context_.scancode = keystroke.scancode;
    context_.vcode = context_.scancode;
    context_.isDownstroke = (keystroke.state & 1) == 0;  // state & 1 == 0 means down

    // Stage 2: Check if capsicain is on
    if (!stage2_CheckCapsicainOn()) {
        result.shouldForward = true;
        return result;
    }

    // Stage 3: Device filtering
    if (!stage3_DeviceFiltering(keystroke.deviceId)) {
        result.shouldForward = true;
        return result;
    }

    // Stage 4: Apple keyboard normalization
    stage4_AppleKeyboardNormalization();

    // Stage 5: Handle messy keys (Print, Pause, Break)
    if (!stage5_MessyKeys()) {
        result.consumed = true;
        return result;
    }

    // Stage 6: Tap detection
    stage6_TapDetection(keystroke);

    // Break tapped state on slow tap
    if (context_.tappedSlow) {
        modifierTracker_.clearAllTapped();
    }

    // Stage 7: Rewire mapping
    if (!stage7_RewireMapping()) {
        result.consumed = true;
        return result;
    }

    // Check if rewired to NOP (disabled key)
    if (context_.vcode == SC_NOP) {
        result.consumed = true;
        return result;
    }

    // Stage 8: Update modifier state
    stage8_ModifierUpdate();

    // Stage 9: Combo matching
    if (!stage9_ComboMatching()) {
        // Combo matched, send the result sequence
        // Convert from capsicain::VKeyEvent to ::VKeyEvent
        for (const auto& evt : context_.resultingVKeyEventSequence) {
            result.outputEvents.push_back({ evt.vcode, evt.isDownstroke });
        }
        return result;
    }

    // Stage 10: Alpha mapping
    stage10_AlphaMapping();

    // Clear tapped state for non-modifiers
    if (!context_.isModifier) {
        modifierTracker_.clearAllTapped();
    }

    // Send the resulting key or sequence
    if (context_.resultingVKeyEventSequence.empty()) {
        // No sequence, send the mapped key
        result.outputEvents.push_back({ context_.vcode, context_.isDownstroke });
    } else {
        // Send the sequence - convert from capsicain::VKeyEvent to ::VKeyEvent
        for (const auto& evt : context_.resultingVKeyEventSequence) {
            result.outputEvents.push_back({ evt.vcode, evt.isDownstroke });
        }
    }

    return result;
}

void KeyProcessingService::reset()
{
    context_.reset();
    modifierTracker_.reset();
}

// ============================================================================
// ProcessingContext
// ============================================================================

void KeyProcessingService::ProcessingContext::reset()
{
    scancode = 0;
    vcode = -1;
    isDownstroke = false;
    isModifier = false;
    tapped = false;
    tappedSlow = false;
    tapHoldMake = false;
    repeat = false;
    resultingVKeyEventSequence.clear();
}

// ============================================================================
// Pipeline Stages
// ============================================================================

void KeyProcessingService::stage1_ResetContext()
{
    context_.reset();
}

bool KeyProcessingService::stage2_CheckCapsicainOn()
{
    return runtime_.isCapsicainOn();
}

bool KeyProcessingService::stage3_DeviceFiltering(int deviceId)
{
    const auto& includeId = runtime_.getIncludeDeviceId();
    const auto& excludeId = runtime_.getExcludeDeviceId();
    const auto& deviceIdStr = runtime_.getDeviceId();

    // Check include filter
    if (!includeId.empty() && deviceIdStr.find(includeId) == std::string::npos) {
        return false;  // Device not included
    }

    // Check exclude filter
    if (!excludeId.empty() && deviceIdStr.find(excludeId) != std::string::npos) {
        return false;  // Device excluded
    }

    return true;
}

void KeyProcessingService::stage4_AppleKeyboardNormalization()
{
    const auto& options = config_.getOptions();

    if (options.flipAltWinOnAppleKeyboards && runtime_.isAppleKeyboard()) {
        switch (context_.vcode) {
            case SC_LALT: context_.vcode = SC_LWIN; break;
            case SC_LWIN: context_.vcode = SC_LALT; break;
            case SC_RALT: context_.vcode = SC_RWIN; break;
            case SC_RWIN: context_.vcode = SC_RALT; break;
        }
        context_.scancode = context_.vcode;  // Simplifies tapping and rewiring
    }
}

bool KeyProcessingService::stage5_MessyKeys()
{
    // Handle special keys that need preprocessing
    // This includes Print Screen, Pause/Break, etc.
    // For now, just allow all keys through
    // TODO: Implement messy key handling if needed
    return true;
}

void KeyProcessingService::stage6_TapDetection(const HardwareKeystroke& stroke)
{
    // Get previous strokes from hardware service for tap detection
    auto prev1 = hardware_.getPreviousStroke1();
    auto prev2 = hardware_.getPreviousStroke2();

    // Convert to KeyEvent format for TapDetector
    KeyEvent currentEvent{
        static_cast<int>(stroke.scancode),
        (stroke.state & 1) == 0
    };
    KeyEvent prev1Event{
        static_cast<int>(prev1.code),
        (prev1.state & 1) == 0
    };
    KeyEvent prev2Event{
        static_cast<int>(prev2.code),
        (prev2.state & 1) == 0
    };

    // Detect tap patterns
    TapResult tapResult = tapDetector_.detect(
        currentEvent, prev1Event, prev2Event,
        stroke.state, prev1.state, prev2.state
    );

    // Store results in context
    context_.tapped = tapResult.tapped;
    context_.tappedSlow = tapResult.tappedSlow;
    context_.tapHoldMake = tapResult.tapHoldMake;
    context_.repeat = tapResult.repeat;
}

bool KeyProcessingService::stage7_RewireMapping()
{
    const auto& mappings = config_.getMappings();

    // Build RewireEntry from rewiremap
    domain::RewireEntry rewireEntry;
    rewireEntry.outKey = mappings.rewiremap[context_.vcode][REWIRE_OUT];
    rewireEntry.tapKey = mappings.rewiremap[context_.scancode][REWIRE_TAP];
    rewireEntry.tapHoldKey = mappings.rewiremap[context_.scancode][REWIRE_TAPHOLD];

    // Build RewireContext
    domain::RewireContext rewireContext;
    rewireContext.scancode = context_.scancode;
    rewireContext.vcode = context_.vcode;
    rewireContext.isDownstroke = context_.isDownstroke;
    rewireContext.isTapped = context_.tapped;
    rewireContext.isTapHoldMake = context_.tapHoldMake;
    rewireContext.activeTapHoldKey = modifierTracker_.getTapHoldKey();

    // Create adapter for modifier query
    class ModifierQuery : public domain::IModifierQuery {
    public:
        explicit ModifierQuery(const ModifierTracker& tracker) : tracker_(tracker) {}
        bool isModifier(domain::VKeyCode vcode) const override {
            return getModifierBitmaskForVcode(vcode) != 0;
        }
        MOD getModifierBitmask(domain::VKeyCode vcode) const override {
            return getModifierBitmaskForVcode(vcode);
        }
    private:
        const ModifierTracker& tracker_;
    };
    ModifierQuery modQuery(modifierTracker_);

    // Apply rewire mapping
    domain::RewireResult rewireResult = keyMapper_.mapRewire(
        rewireContext, rewireEntry, &modQuery
    );

    // Apply results
    context_.vcode = rewireResult.outputKey;
    context_.isModifier = rewireResult.isModifier;

    // Add generated events to sequence
    for (const auto& evt : rewireResult.eventSequence) {
        context_.resultingVKeyEventSequence.push_back({
            static_cast<int>(evt.keyCode),
            evt.isDown
        });
    }

    // Update modifier state based on rewire result
    if (rewireResult.modifiersToClear != 0) {
        // Clear specific modifiers
        // Note: ModifierTracker doesn't have a direct method for this
        // We'll need to handle this in a future refactoring
    }
    if (rewireResult.tappedToClear != 0) {
        modifierTracker_.clearAllTapped();
    }

    // Update tap-hold key
    if (rewireResult.newTapHoldKey != 0) {
        if (rewireResult.newTapHoldKey == -1) {
            modifierTracker_.clearTapHoldKey();
        } else {
            modifierTracker_.setTapHoldKey(rewireResult.newTapHoldKey);
        }
    }

    // Check if key should be suppressed
    if (rewireResult.shouldNop) {
        return false;
    }

    return true;
}

void KeyProcessingService::stage8_ModifierUpdate()
{
    // Update modifier tracker with current key state
    modifierTracker_.update(context_.vcode, context_.isDownstroke, context_.tapped);
}

bool KeyProcessingService::stage9_ComboMatching()
{
    const auto& mappings = config_.getMappings();

    // Build ComboMatchContext
    domain::ComboMatchContext comboContext;
    comboContext.currentKey = context_.vcode;
    comboContext.modifiersDown = modifierTracker_.getDownMask();
    comboContext.modifiersTapped = modifierTracker_.getTappedMask();
    comboContext.deviceMask = 0;  // TODO: Calculate from device ID if needed
    comboContext.activeDeadkey = modifierTracker_.getDeadkey();

    // Match combos based on keystroke direction
    domain::ComboMatchResult comboResult;

    if (context_.isDownstroke) {
        auto it1 = mappings.convertedCombos.find(INI_TAG_COMBOS);
        auto it2 = mappings.convertedCombos.find(INI_TAG_REPEATCOMBOS);

        const auto& combos = (it1 != mappings.convertedCombos.end())
            ? it1->second
            : std::vector<domain::ComboRule>();
        const auto& repeatCombos = (it2 != mappings.convertedCombos.end())
            ? it2->second
            : std::vector<domain::ComboRule>();

        comboResult = comboMatcher_.matchDownstroke(
            combos,
            repeatCombos,
            comboContext,
            context_.repeat
        );
    } else {
        auto it1 = mappings.convertedCombos.find(INI_TAG_UPCOMBOS);
        auto it2 = mappings.convertedCombos.find(INI_TAG_TAPCOMBOS);
        auto it3 = mappings.convertedCombos.find(INI_TAG_SLOWCOMBOS);

        const auto& upCombos = (it1 != mappings.convertedCombos.end())
            ? it1->second
            : std::vector<domain::ComboRule>();
        const auto& tapCombos = (it2 != mappings.convertedCombos.end())
            ? it2->second
            : std::vector<domain::ComboRule>();
        const auto& slowCombos = (it3 != mappings.convertedCombos.end())
            ? it3->second
            : std::vector<domain::ComboRule>();

        comboResult = comboMatcher_.matchUpstroke(
            upCombos,
            tapCombos,
            slowCombos,
            comboContext,
            context_.tapped,
            context_.tappedSlow
        );
    }

    // Apply combo result
    if (comboResult.matched) {
        // Convert ComboKeyEvent sequence to VKeyEvent
        context_.resultingVKeyEventSequence.clear();
        for (const auto& evt : comboResult.resultSequence) {
            context_.resultingVKeyEventSequence.push_back({
                static_cast<int>(evt.keyCode),
                evt.isDown
            });
        }

        // Clear tapped state if needed
        if (comboResult.shouldClearTapped) {
            modifierTracker_.clearAllTapped();
        }

        // Clear deadkey for non-modifier keys
        if (!context_.isModifier) {
            modifierTracker_.clearDeadkey();
        }

        return false;  // Combo matched, don't continue to alpha mapping
    }

    // Clear deadkey for non-modifier keys even if no combo matched
    if (!context_.isModifier) {
        modifierTracker_.clearDeadkey();
    }

    return true;  // Continue to alpha mapping
}

bool KeyProcessingService::stage10_AlphaMapping()
{
    const auto& options = config_.getOptions();
    const auto& mappings = config_.getMappings();

    // Build AlphaMapOptions
    domain::AlphaMapOptions alphaOptions;
    alphaOptions.flipZY = options.flipZY;
    alphaOptions.ctrlWinBlocksAlphaMapping = options.lControlLWinBlocksAlphaMapping;

    // Apply alpha mapping
    domain::AlphaMapResult alphaResult = keyMapper_.mapAlpha(
        context_.vcode,
        mappings.alphamap.data(),
        alphaOptions,
        context_.isModifier,
        modifierTracker_.isLCtrlDown(),
        modifierTracker_.isLWinDown()
    );

    // Update vcode with mapped result
    context_.vcode = alphaResult.mappedKey;

    return alphaResult.mappedKey != context_.vcode;  // Return true if mapping was applied
}

} // namespace services
} // namespace capsicain
