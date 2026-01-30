#include "services/KeyProcessingService.h"
#include "services/ConfigurationService.h"
#include "services/RuntimeStateService.h"
#include "services/HardwareService.h"
#include "services/UIService.h"

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
    // TODO: Implement full processing pipeline
    // For now, return a forward result to maintain current behavior
    ProcessResult result;
    result.shouldForward = true;
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
// Pipeline Stages (stubs for now)
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
    // TODO: Implement device filtering logic
    return true;
}

void KeyProcessingService::stage4_AppleKeyboardNormalization()
{
    // TODO: Implement Apple keyboard normalization
}

bool KeyProcessingService::stage5_MessyKeys()
{
    // TODO: Handle Print, Pause, Break keys
    return true;
}

void KeyProcessingService::stage6_TapDetection(const HardwareKeystroke& stroke)
{
    // TODO: Implement tap detection using TapDetector
}

bool KeyProcessingService::stage7_RewireMapping()
{
    // TODO: Implement rewire mapping using KeyMapper
    return true;
}

void KeyProcessingService::stage8_ModifierUpdate()
{
    // TODO: Update ModifierTracker with current key state
}

bool KeyProcessingService::stage9_ComboMatching()
{
    // TODO: Implement combo matching using ComboMatcher
    return true;
}

bool KeyProcessingService::stage10_AlphaMapping()
{
    // TODO: Implement alpha mapping using KeyMapper
    return false;
}

} // namespace services
} // namespace capsicain
