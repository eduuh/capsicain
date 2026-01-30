#pragma once

#include <string>
#include <array>
#include "platform/constants.h"

// Forward declaration
class ConsoleUI;

namespace capsicain {
namespace services {

/**
 * @brief UIService encapsulates UI-related functionality and label management
 *
 * Responsibilities:
 * - Owns the PRETTY_VK_LABELS array (human-readable key names)
 * - Wraps ConsoleUI with convenience methods
 * - Provides formatted label access
 *
 * Usage:
 *   UIService ui(consoleUI);
 *   ui.initialize();  // Populates PRETTY_VK_LABELS
 *   std::string label = ui.getLabel(SC_A);  // "A"
 *   std::string padded = ui.getLabelPadded(SC_ESC, 10);  // "       ESC"
 */
class UIService {
public:
    /**
     * @brief Construct UIService with reference to ConsoleUI
     * @param consoleUI Console output handler (non-owning reference)
     */
    explicit UIService(ConsoleUI& consoleUI) noexcept;

    /**
     * @brief Initialize the service (populate PRETTY_VK_LABELS)
     * Must be called once during application startup
     */
    void initialize();

    /**
     * @brief Get human-readable label for a virtual key code
     * @param vcode Virtual key code (0 to MAX_VCODES-1)
     * @return Label string (e.g., "A", "ESC", "LCTRL")
     */
    [[nodiscard]] const std::string& getLabel(int vcode) const noexcept;

    /**
     * @brief Get padded label for aligned output
     * @param vcode Virtual key code
     * @param resultLength Desired total length (left-padded with spaces)
     * @return Padded label string
     */
    [[nodiscard]] std::string getLabelPadded(int vcode, int resultLength) const;

    /**
     * @brief Get raw access to labels array (for legacy code)
     * @return Pointer to internal array
     */
    [[nodiscard]] std::string* getLabelsArray() noexcept {
        return labels_.data();
    }

    /**
     * @brief Access underlying ConsoleUI
     * @return Reference to ConsoleUI instance
     */
    [[nodiscard]] ConsoleUI& getConsoleUI() noexcept {
        return consoleUI_;
    }

    // Convenience methods wrapping ConsoleUI
    void printHeader(const std::string& text) const;
    void printStatus(const std::string& text) const;
    void printDebug(const std::string& text) const;

private:
    ConsoleUI& consoleUI_;  // Non-owning reference
    std::array<std::string, MAX_VCODES> labels_;  // Replaces global PRETTY_VK_LABELS
    bool initialized_ = false;
};

} // namespace services
} // namespace capsicain
