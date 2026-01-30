#pragma once

#include <string>
#include <vector>

namespace capsicain {
namespace services {

/**
 * @brief ConfigurationService encapsulates configuration data and INI file handling
 *
 * Phase 1: Manages sanitized INI content
 * Future phases will add: GlobalSettings, RuntimeOptions, MappingData
 *
 * Responsibilities:
 * - Load and parse capsicain.ini file
 * - Provide access to sanitized INI content
 * - Extract sections and tagged lines
 *
 * Usage:
 *   ConfigurationService config;
 *   if (config.loadIniFile()) {
 *       auto section = config.getSection("Config0");
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

private:
    std::vector<std::string> sanitizedIniContent_;  // Replaces global sanitizedIniContent
};

} // namespace services
} // namespace capsicain
