#pragma once

#include <string>
#include <array>
#include <vector>
#include <set>
#include "platform/constants.h"
#include "core/Types.h"
#include "core/scancodes.h"

namespace capsicain {
namespace services {

/**
 * @brief RuntimeStateService encapsulates application runtime state
 *
 * Replaces the global `globalState` variable with proper encapsulation.
 * Manages:
 * - Application on/off state
 * - Active configuration tracking
 * - Device identification and filtering
 * - Key tracking (sent, temp released, hold mappings)
 * - Macro recording state
 * - ESC key state
 * - Secret sequence state
 *
 * This is the most stateful service - it tracks dynamic application state
 * that changes during runtime based on user input and configuration switching.
 */
class RuntimeStateService {
public:
    RuntimeStateService() noexcept;

    // ========================================================================
    // Application State
    // ========================================================================

    /**
     * @brief Check if Capsicain is currently active
     * @return true if Capsicain is processing keys, false if passthrough mode
     */
    [[nodiscard]] bool isCapsicainOn() const noexcept;

    /**
     * @brief Set Capsicain on/off state
     * @param on true to enable key processing, false for passthrough
     */
    void setCapsicainOn(bool on) noexcept;

    /**
     * @brief Toggle Capsicain on/off state
     * @return New state after toggle
     */
    bool toggleCapsicainOn() noexcept;

    // ========================================================================
    // Configuration Tracking
    // ========================================================================

    /**
     * @brief Get currently active configuration index
     * @return Config index (0=disabled, 1-9=active configs)
     */
    [[nodiscard]] int getActiveConfig() const noexcept;

    /**
     * @brief Get currently active configuration name
     * @return Config name string
     */
    [[nodiscard]] const std::string& getActiveConfigName() const noexcept;

    /**
     * @brief Switch to a new configuration
     * @param config Config index (0-9)
     * @param configName Human-readable name for the config
     */
    void setActiveConfig(int config, const std::string& configName) noexcept;

    /**
     * @brief Get previous configuration index (for CONFIGPREVIOUS function)
     * @return Previous config index
     */
    [[nodiscard]] int getPreviousConfig() const noexcept;

    /**
     * @brief Set previous configuration index
     * @param config Previous config index
     */
    void setPreviousConfig(int config) noexcept;

    // ========================================================================
    // Device Identification
    // ========================================================================

    /**
     * @brief Set device identification strings
     * @param keyboardId Hardware ID of keyboard device
     * @param isAppleKeyboard Whether device is an Apple keyboard
     */
    void setDeviceInfo(const std::string& keyboardId, bool isAppleKeyboard) noexcept;

    /**
     * @brief Get keyboard device hardware ID
     * @return Device ID string
     */
    [[nodiscard]] const std::string& getDeviceId() const noexcept;

    /**
     * @brief Check if current device is an Apple keyboard
     * @return true if Apple keyboard detected
     */
    [[nodiscard]] bool isAppleKeyboard() const noexcept;

    // ========================================================================
    // Device Filtering
    // ========================================================================

    /**
     * @brief Set device filter IDs for inclusion/exclusion
     * @param includeId Device ID to include (empty = no filter)
     * @param excludeId Device ID to exclude (empty = no filter)
     */
    void setDeviceFilter(const std::string& includeId, const std::string& excludeId) noexcept;

    /**
     * @brief Get device ID to include (whitelist)
     * @return Include device ID string
     */
    [[nodiscard]] const std::string& getIncludeDeviceId() const noexcept;

    /**
     * @brief Get device ID to exclude (blacklist)
     * @return Exclude device ID string
     */
    [[nodiscard]] const std::string& getExcludeDeviceId() const noexcept;

    // ========================================================================
    // Key Tracking (Keys Sent to OS)
    // ========================================================================

    /**
     * @brief Record that a key was sent down to the OS
     * @param vcode Virtual key code (must be 0-255)
     */
    void recordKeySent(int vcode) noexcept;

    /**
     * @brief Record that a key was released (sent up to OS)
     * @param vcode Virtual key code (must be 0-255)
     */
    void recordKeyReleased(int vcode) noexcept;

    /**
     * @brief Check if a key is currently down (sent to OS)
     * @param vcode Virtual key code (must be 0-255)
     * @return true if key is down
     */
    [[nodiscard]] bool isKeySent(int vcode) const noexcept;

    /**
     * @brief Get count of keys currently down
     * @return Number of keys down
     */
    [[nodiscard]] int getKeysDownSentCounter() const noexcept;

    /**
     * @brief Clear all keys down state
     */
    void clearKeysDownSent() noexcept;

    // ========================================================================
    // Temp Released Keys (for Alt-Numpad combos)
    // ========================================================================

    /**
     * @brief Record that a key was temporarily released
     * @param vcode Virtual key code (must be 0-255)
     */
    void recordTempReleased(int vcode) noexcept;

    /**
     * @brief Check if a key was temporarily released
     * @param vcode Virtual key code (must be 0-255)
     * @return true if key was temp released
     */
    [[nodiscard]] bool isTempReleased(int vcode) const noexcept;

    /**
     * @brief Clear temp released state for a key
     * @param vcode Virtual key code (must be 0-255)
     */
    void clearTempReleased(int vcode) noexcept;

    /**
     * @brief Clear all temp released keys
     */
    void clearAllTempReleased() noexcept;

    // ========================================================================
    // Hold() Key Tracking
    // ========================================================================

    /**
     * @brief Record a hold() replacement key for a physical key
     * @param physicalKey Physical key code
     * @param holdKey Replacement key code
     */
    void recordHoldKey(int physicalKey, int holdKey) noexcept;

    /**
     * @brief Get all hold() replacement keys for a physical key
     * @param physicalKey Physical key code
     * @return Set of replacement key codes
     */
    [[nodiscard]] const std::set<int>& getHoldKeys(int physicalKey) const noexcept;

    /**
     * @brief Clear all hold() keys for a physical key
     * @param physicalKey Physical key code
     */
    void clearHoldKeys(int physicalKey) noexcept;

    // ========================================================================
    // Macro Recording
    // ========================================================================

    /**
     * @brief Start recording a macro
     * @param macroIndex Macro slot (0-9)
     */
    void startRecording(int macroIndex) noexcept;

    /**
     * @brief Stop recording current macro
     * @return Macro index that was recording, or -1 if not recording
     */
    int stopRecording() noexcept;

    /**
     * @brief Check if currently recording a macro
     * @return true if recording
     */
    [[nodiscard]] bool isRecording() const noexcept;

    /**
     * @brief Get current recording macro index
     * @return Macro index (0-9), or -1 if not recording
     */
    [[nodiscard]] int getRecordingMacroIndex() const noexcept;

    /**
     * @brief Record a keystroke in the current macro
     * @param keyEvent Key event to record
     */
    void recordKeystroke(const VKeyEvent& keyEvent) noexcept;

    /**
     * @brief Get recorded macro
     * @param macroIndex Macro slot (0-9)
     * @return Vector of recorded key events
     */
    [[nodiscard]] const std::vector<VKeyEvent>& getMacro(int macroIndex) const noexcept;

    /**
     * @brief Clear a macro
     * @param macroIndex Macro slot (0-9)
     */
    void clearMacro(int macroIndex) noexcept;

    /**
     * @brief Get mutable reference to macro for editing
     * @param macroIndex Macro slot (0-9)
     * @return Mutable reference to macro vector
     */
    std::vector<VKeyEvent>& getMacroMutable(int macroIndex) noexcept;

    // ========================================================================
    // ESC Key State
    // ========================================================================

    /**
     * @brief Check if real ESC key is currently down
     * @return true if ESC is down
     */
    [[nodiscard]] bool isRealEscapeDown() const noexcept;

    /**
     * @brief Set real ESC key down state
     * @param down true if ESC is down
     */
    void setRealEscapeDown(bool down) noexcept;

    // ========================================================================
    // Secret Sequence State
    // ========================================================================

    /**
     * @brief Check if recording a secret sequence
     * @return true if recording
     */
    [[nodiscard]] bool isSecretSequenceRecording() const noexcept;

    /**
     * @brief Set secret sequence recording state
     * @param recording true to start recording
     */
    void setSecretSequenceRecording(bool recording) noexcept;

    /**
     * @brief Check if playing back a secret sequence
     * @return true if playing back
     */
    [[nodiscard]] bool isSecretSequencePlayback() const noexcept;

    /**
     * @brief Set secret sequence playback state
     * @param playback true if playing back
     */
    void setSecretSequencePlayback(bool playback) noexcept;

    // ========================================================================
    // Reset
    // ========================================================================

    /**
     * @brief Reset all runtime state to defaults
     */
    void reset() noexcept;

private:
    // Application state
    bool capsicainOn_ = true;

    // Configuration tracking
    int activeConfig_ = 0;
    std::string activeConfigName_ = DEFAULT_ACTIVE_CONFIG_NAME;
    int previousConfig_ = 1;

    // ESC state
    bool realEscapeIsDown_ = false;

    // Device identification
    std::string deviceIdKeyboard_;
    std::string includeDeviceId_;
    std::string excludeDeviceId_;
    bool deviceIsAppleKeyboard_ = false;

    // Key tracking
    int keysDownSentCounter_ = 0;
    std::array<bool, 256> keysDownSent_{};
    std::array<bool, 256> keysDownTempReleased_{};
    std::array<std::set<int>, VK_MAX> holdKeys_;

    // Secret sequence
    bool secretSequenceRecording_ = false;
    bool secretSequencePlayback_ = false;

    // Macro recording
    int recordingMacro_ = -1;  // -1 = not recording
    std::array<std::vector<VKeyEvent>, MAX_NUM_MACROS> recordedMacros_;

    // Empty set for getHoldKeys() return value
    static const std::set<int> emptySet_;
};

} // namespace services
} // namespace capsicain
