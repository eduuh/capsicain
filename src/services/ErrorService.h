#pragma once

#include <string>

namespace capsicain {
namespace services {

/**
 * @brief ErrorService encapsulates error logging functionality
 *
 * Responsibilities:
 * - Accumulates error messages during application execution
 * - Provides error log access for display or debugging
 *
 * Usage:
 *   ErrorService errors;
 *   errors.logError("Configuration file not found");
 *   if (errors.hasErrors()) {
 *       std::cout << errors.getErrorLog();
 *   }
 */
class ErrorService {
public:
    ErrorService() = default;

    /**
     * @brief Log an error message
     * @param message Error message to log
     *
     * Outputs to console immediately and appends to internal log
     */
    void logError(const std::string& message);

    /**
     * @brief Get accumulated error log
     * @return All error messages concatenated with line breaks
     */
    [[nodiscard]] const std::string& getErrorLog() const noexcept {
        return errorLog_;
    }

    /**
     * @brief Check if any errors have been logged
     * @return true if error log is not empty
     */
    [[nodiscard]] bool hasErrors() const noexcept {
        return !errorLog_.empty();
    }

    /**
     * @brief Clear all logged errors
     */
    void clearErrors() noexcept {
        errorLog_.clear();
    }

private:
    std::string errorLog_;  // Replaces global errorLog
};

} // namespace services
} // namespace capsicain
