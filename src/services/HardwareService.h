#pragma once

#include "platform/interception.h"
#include <optional>

namespace capsicain {
namespace services {

/**
 * @brief HardwareService encapsulates Interception hardware I/O
 *
 * Responsibilities:
 * - Manage Interception context and device state
 * - Wait for and receive keystrokes
 * - Send keystrokes to OS
 * - Track device information (Apple keyboards, etc.)
 * - Maintain keystroke history for tap detection
 *
 * Usage:
 *   HardwareService hw;
 *   hw.initialize();
 *   while (auto keystroke = hw.waitForKey(2)) {
 *       // Process keystroke
 *       hw.sendCurrentKey();
 *   }
 *   hw.shutdown();
 */
class HardwareService {
public:
    /**
     * @brief Keystroke data returned by waitForKey()
     */
    struct KeyStroke {
        InterceptionDevice device;
        InterceptionKeyStroke stroke;
    };

    HardwareService() = default;
    ~HardwareService() { shutdown(); }

    // Prevent copying
    HardwareService(const HardwareService&) = delete;
    HardwareService& operator=(const HardwareService&) = delete;

    /**
     * @brief Initialize Interception context and filters
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Shutdown and destroy Interception context
     */
    void shutdown();

    /**
     * @brief Wait for a keystroke
     * @param timeoutMS Timeout in milliseconds
     * @return KeyStroke if received, std::nullopt if timeout
     */
    [[nodiscard]] std::optional<KeyStroke> waitForKey(int timeoutMS);

    /**
     * @brief Send the current keystroke back to OS
     */
    void sendCurrentKey();

    /**
     * @brief Send a custom keystroke
     * @param device Device to send to
     * @param stroke Keystroke to send
     */
    void sendKey(InterceptionDevice device, const InterceptionKeyStroke& stroke);

    /**
     * @brief Get current device
     */
    [[nodiscard]] InterceptionDevice getCurrentDevice() const noexcept {
        return currentDevice_;
    }

    /**
     * @brief Get previous device
     */
    [[nodiscard]] InterceptionDevice getPreviousDevice() const noexcept {
        return previousDevice_;
    }

    /**
     * @brief Get current keystroke
     */
    [[nodiscard]] const InterceptionKeyStroke& getCurrentStroke() const noexcept {
        return currentStroke_;
    }

    /**
     * @brief Get previous keystroke 1 (for tap detection)
     */
    [[nodiscard]] const InterceptionKeyStroke& getPreviousStroke1() const noexcept {
        return previousStroke1_;
    }

    /**
     * @brief Get previous keystroke 2 (for tap detection)
     */
    [[nodiscard]] const InterceptionKeyStroke& getPreviousStroke2() const noexcept {
        return previousStroke2_;
    }

    /**
     * @brief Get last mouse device
     */
    [[nodiscard]] InterceptionDevice getLastMouse() const noexcept {
        return lastMouse_;
    }

    /**
     * @brief Get last keyboard device
     */
    [[nodiscard]] InterceptionDevice getLastKeyboard() const noexcept {
        return lastKeyboard_;
    }

    /**
     * @brief Get new keyboard counter
     */
    [[nodiscard]] int getNewKeyboardCounter() const noexcept {
        return newKeyboardCounter_;
    }

    /**
     * @brief Increment new keyboard counter
     */
    void incrementNewKeyboardCounter() noexcept {
        ++newKeyboardCounter_;
    }

    /**
     * @brief Set filter for mouse input
     * @param enable true to enable mouse, false to disable
     */
    void setMouseFilter(bool enable);

private:
    // Interception context
    InterceptionContext context_ = nullptr;

    // Device tracking
    InterceptionDevice currentDevice_ = 0;
    InterceptionDevice previousDevice_ = 0;
    InterceptionDevice lastMouse_ = 0;
    InterceptionDevice lastKeyboard_ = 0;
    int newKeyboardCounter_ = 0;

    // Keystroke history (for tap detection)
    InterceptionKeyStroke currentStroke_ = { 0, 0 };
    InterceptionKeyStroke previousStroke1_ = { 0, 0 };
    InterceptionKeyStroke previousStroke2_ = { 0, 0 };

    // Update keystroke history
    void updateHistory(const InterceptionKeyStroke& stroke);
};

} // namespace services
} // namespace capsicain
