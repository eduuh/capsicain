#include "services/RuntimeStateService.h"

namespace capsicain {
namespace services {

// Static member initialization
const std::set<int> RuntimeStateService::emptySet_;

RuntimeStateService::RuntimeStateService() noexcept
{
    // All members have default initialization in header
}

// ============================================================================
// Application State
// ============================================================================

bool RuntimeStateService::isCapsicainOn() const noexcept
{
    return capsicainOn_;
}

void RuntimeStateService::setCapsicainOn(bool on) noexcept
{
    capsicainOn_ = on;
}

bool RuntimeStateService::toggleCapsicainOn() noexcept
{
    capsicainOn_ = !capsicainOn_;
    return capsicainOn_;
}

// ============================================================================
// Configuration Tracking
// ============================================================================

int RuntimeStateService::getActiveConfig() const noexcept
{
    return activeConfig_;
}

const std::string& RuntimeStateService::getActiveConfigName() const noexcept
{
    return activeConfigName_;
}

void RuntimeStateService::setActiveConfig(int config, const std::string& configName) noexcept
{
    activeConfig_ = config;
    activeConfigName_ = configName;
}

int RuntimeStateService::getPreviousConfig() const noexcept
{
    return previousConfig_;
}

void RuntimeStateService::setPreviousConfig(int config) noexcept
{
    previousConfig_ = config;
}

// ============================================================================
// Device Identification
// ============================================================================

void RuntimeStateService::setDeviceInfo(const std::string& keyboardId, bool isAppleKeyboard) noexcept
{
    deviceIdKeyboard_ = keyboardId;
    deviceIsAppleKeyboard_ = isAppleKeyboard;
}

const std::string& RuntimeStateService::getDeviceId() const noexcept
{
    return deviceIdKeyboard_;
}

bool RuntimeStateService::isAppleKeyboard() const noexcept
{
    return deviceIsAppleKeyboard_;
}

// ============================================================================
// Device Filtering
// ============================================================================

void RuntimeStateService::setDeviceFilter(const std::string& includeId, const std::string& excludeId) noexcept
{
    includeDeviceId_ = includeId;
    excludeDeviceId_ = excludeId;
}

const std::string& RuntimeStateService::getIncludeDeviceId() const noexcept
{
    return includeDeviceId_;
}

const std::string& RuntimeStateService::getExcludeDeviceId() const noexcept
{
    return excludeDeviceId_;
}

// ============================================================================
// Key Tracking (Keys Sent to OS)
// ============================================================================

void RuntimeStateService::recordKeySent(int vcode) noexcept
{
    if (vcode >= 0 && vcode < 256) {
        if (!keysDownSent_[vcode]) {
            keysDownSent_[vcode] = true;
            keysDownSentCounter_++;
        }
    }
}

void RuntimeStateService::recordKeyReleased(int vcode) noexcept
{
    if (vcode >= 0 && vcode < 256) {
        if (keysDownSent_[vcode]) {
            keysDownSent_[vcode] = false;
            keysDownSentCounter_--;
        }
    }
}

bool RuntimeStateService::isKeySent(int vcode) const noexcept
{
    if (vcode >= 0 && vcode < 256) {
        return keysDownSent_[vcode];
    }
    return false;
}

int RuntimeStateService::getKeysDownSentCounter() const noexcept
{
    return keysDownSentCounter_;
}

void RuntimeStateService::clearKeysDownSent() noexcept
{
    keysDownSent_.fill(false);
    keysDownSentCounter_ = 0;
}

// ============================================================================
// Temp Released Keys (for Alt-Numpad combos)
// ============================================================================

void RuntimeStateService::recordTempReleased(int vcode) noexcept
{
    if (vcode >= 0 && vcode < 256) {
        keysDownTempReleased_[vcode] = true;
    }
}

bool RuntimeStateService::isTempReleased(int vcode) const noexcept
{
    if (vcode >= 0 && vcode < 256) {
        return keysDownTempReleased_[vcode];
    }
    return false;
}

void RuntimeStateService::clearTempReleased(int vcode) noexcept
{
    if (vcode >= 0 && vcode < 256) {
        keysDownTempReleased_[vcode] = false;
    }
}

void RuntimeStateService::clearAllTempReleased() noexcept
{
    keysDownTempReleased_.fill(false);
}

// ============================================================================
// Hold() Key Tracking
// ============================================================================

void RuntimeStateService::recordHoldKey(int physicalKey, int holdKey) noexcept
{
    if (physicalKey >= 0 && physicalKey < VK_MAX) {
        holdKeys_[physicalKey].insert(holdKey);
    }
}

const std::set<int>& RuntimeStateService::getHoldKeys(int physicalKey) const noexcept
{
    if (physicalKey >= 0 && physicalKey < VK_MAX) {
        return holdKeys_[physicalKey];
    }
    return emptySet_;
}

void RuntimeStateService::clearHoldKeys(int physicalKey) noexcept
{
    if (physicalKey >= 0 && physicalKey < VK_MAX) {
        holdKeys_[physicalKey].clear();
    }
}

// ============================================================================
// Macro Recording
// ============================================================================

void RuntimeStateService::startRecording(int macroIndex) noexcept
{
    if (macroIndex >= 0 && macroIndex < MAX_NUM_MACROS) {
        recordingMacro_ = macroIndex;
        recordedMacros_[macroIndex].clear();
    }
}

int RuntimeStateService::stopRecording() noexcept
{
    int index = recordingMacro_;
    recordingMacro_ = -1;
    return index;
}

bool RuntimeStateService::isRecording() const noexcept
{
    return recordingMacro_ >= 0;
}

int RuntimeStateService::getRecordingMacroIndex() const noexcept
{
    return recordingMacro_;
}

void RuntimeStateService::recordKeystroke(const VKeyEvent& keyEvent) noexcept
{
    if (recordingMacro_ >= 0 && recordingMacro_ < MAX_NUM_MACROS) {
        recordedMacros_[recordingMacro_].push_back(keyEvent);
    }
}

const std::vector<VKeyEvent>& RuntimeStateService::getMacro(int macroIndex) const noexcept
{
    static const std::vector<VKeyEvent> empty;
    if (macroIndex >= 0 && macroIndex < MAX_NUM_MACROS) {
        return recordedMacros_[macroIndex];
    }
    return empty;
}

void RuntimeStateService::clearMacro(int macroIndex) noexcept
{
    if (macroIndex >= 0 && macroIndex < MAX_NUM_MACROS) {
        recordedMacros_[macroIndex].clear();
    }
}

std::vector<VKeyEvent>& RuntimeStateService::getMacroMutable(int macroIndex) noexcept
{
    static std::vector<VKeyEvent> empty;
    if (macroIndex >= 0 && macroIndex < MAX_NUM_MACROS) {
        return recordedMacros_[macroIndex];
    }
    return empty;
}

// ============================================================================
// ESC Key State
// ============================================================================

bool RuntimeStateService::isRealEscapeDown() const noexcept
{
    return realEscapeIsDown_;
}

void RuntimeStateService::setRealEscapeDown(bool down) noexcept
{
    realEscapeIsDown_ = down;
}

// ============================================================================
// Secret Sequence State
// ============================================================================

bool RuntimeStateService::isSecretSequenceRecording() const noexcept
{
    return secretSequenceRecording_;
}

void RuntimeStateService::setSecretSequenceRecording(bool recording) noexcept
{
    secretSequenceRecording_ = recording;
}

bool RuntimeStateService::isSecretSequencePlayback() const noexcept
{
    return secretSequencePlayback_;
}

void RuntimeStateService::setSecretSequencePlayback(bool playback) noexcept
{
    secretSequencePlayback_ = playback;
}

// ============================================================================
// Reset
// ============================================================================

void RuntimeStateService::reset() noexcept
{
    capsicainOn_ = true;
    activeConfig_ = 0;
    activeConfigName_ = DEFAULT_ACTIVE_CONFIG_NAME;
    previousConfig_ = 1;
    realEscapeIsDown_ = false;
    deviceIdKeyboard_.clear();
    includeDeviceId_.clear();
    excludeDeviceId_.clear();
    deviceIsAppleKeyboard_ = false;
    keysDownSentCounter_ = 0;
    keysDownSent_.fill(false);
    keysDownTempReleased_.fill(false);
    for (auto& holdSet : holdKeys_) {
        holdSet.clear();
    }
    secretSequenceRecording_ = false;
    secretSequencePlayback_ = false;
    recordingMacro_ = -1;
    for (auto& macro : recordedMacros_) {
        macro.clear();
    }
}

} // namespace services
} // namespace capsicain
