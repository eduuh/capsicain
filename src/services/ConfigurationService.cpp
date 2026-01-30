#include "services/ConfigurationService.h"
#include "legacy/configUtils.h"

namespace capsicain {
namespace services {

bool ConfigurationService::loadIniFile()
{
    return readSanitizeIniFile(sanitizedIniContent_);
}

bool ConfigurationService::reloadIniFile()
{
    sanitizedIniContent_.clear();
    return readSanitizeIniFile(sanitizedIniContent_);
}

std::vector<std::string> ConfigurationService::getSection(const std::string& sectionName) const
{
    return getSectionFromIni(sectionName, sanitizedIniContent_);
}

std::vector<std::string> ConfigurationService::getTaggedLines(const std::string& tag) const
{
    return getTaggedLinesFromIni(tag, sanitizedIniContent_);
}

} // namespace services
} // namespace capsicain
