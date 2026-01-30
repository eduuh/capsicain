#pragma once

#include <string>
#include <vector>
#include "core/Types.h"

namespace capsicain {
namespace services {

/**
 * @brief ConfigurationService encapsulates configuration data and INI file handling
 *
 * Phase 1: Manages sanitized INI content
 * Phase 2: Manages GlobalSettings and RuntimeOptions
 * Future phases will add: MappingData
 *
 * Responsibilities:
 * - Load and parse capsicain.ini file
 * - Provide access to sanitized INI content
 * - Extract sections and tagged lines
 * - Manage global settings (read-only after INI load)
 * - Manage runtime options (mutable via ESC commands)
 *
 * Usage:
 *   ConfigurationService config;
 *   if (config.loadIniFile()) {
 *       auto& settings = config.getGlobalSettings();
 *       config.getOptions().debug = true;  // Toggle option
 *   }
 */
class ConfigurationService {
public:
    ConfigurationService() = default;

    /**
     * @brief Load and sanitize INI file
     * @return true if successful, false on error
     *
     * Reads capsicain.ini, removes comments, trims whitespace
     */
    bool loadIniFile();

    /**
     * @brief Reload INI file (clear and load again)
     * @return true if successful, false on error
     */
    bool reloadIniFile();

    /**
     * @brief Get sanitized INI content
     * @return Reference to INI lines vector
     */
    [[nodiscard]] const std::vector<std::string>& getIniContent() const noexcept {
        return sanitizedIniContent_;
    }

    /**
     * @brief Get mutable access to INI content (for legacy code)
     * @return Reference to INI lines vector
     */
    [[nodiscard]] std::vector<std::string>& getIniContentMutable() noexcept {
        return sanitizedIniContent_;
    }

    /**
     * @brief Get a specific section from INI
     * @param sectionName Name of section (e.g., "Config0")
     * @return Lines belonging to that section
     */
    [[nodiscard]] std::vector<std::string> getSection(const std::string& sectionName) const;

    /**
     * @brief Get lines with a specific tag from INI
     * @param tag Tag to search for (e.g., "Global")
     * @return Lines with that tag
     */
    [[nodiscard]] std::vector<std::string> getTaggedLines(const std::string& tag) const;

    /**
     * @brief Check if INI is loaded
     * @return true if INI content is not empty
     */
    [[nodiscard]] bool isLoaded() const noexcept {
        return !sanitizedIniContent_.empty();
    }

    /**
     * @brief Clear all configuration data
     */
    void clear() noexcept {
        sanitizedIniContent_.clear();
    }

    // Phase 2: GlobalSettings and RuntimeOptions accessors

    /**
     * @brief Get global settings (read-only)
     * @return Const reference to global settings
     */
    [[nodiscard]] const capsicain::GlobalSettings& getGlobalSettings() const noexcept {
        return globalSettings_;
    }

    /**
     * @brief Get mutable global settings (for initialization)
     * @return Reference to global settings
     */
    [[nodiscard]] capsicain::GlobalSettings& getGlobalSettingsMutable() noexcept {
        return globalSettings_;
    }

    /**
     * @brief Get runtime options (const)
     * @return Const reference to runtime options
     */
    [[nodiscard]] const capsicain::RuntimeOptions& getOptions() const noexcept {
        return options_;
    }

    /**
     * @brief Get mutable runtime options (for ESC command toggling)
     * @return Reference to runtime options
     */
    [[nodiscard]] capsicain::RuntimeOptions& getOptionsMutable() noexcept {
        return options_;
    }

private:
    std::vector<std::string> sanitizedIniContent_;  // Replaces global sanitizedIniContent
    capsicain::GlobalSettings globalSettings_;      // Replaces global globals
    capsicain::RuntimeOptions options_;              // Replaces global options
};

} // namespace services
} // namespace capsicain
