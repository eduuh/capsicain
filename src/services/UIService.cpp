#include "services/UIService.h"
#include "ui/ConsoleUI.h"
#include "core/scancodes.h"
#include <iostream>

namespace capsicain {
namespace services {

UIService::UIService(ConsoleUI& consoleUI) noexcept
    : consoleUI_(consoleUI)
{
}

void UIService::initialize()
{
    if (initialized_) {
        return;  // Already initialized
    }

    // Populate labels using existing function from scancodes.cpp
    defineAllPrettyVKLabels(labels_.data());
    initialized_ = true;
}

const std::string& UIService::getLabel(int vcode) const noexcept
{
    static const std::string empty;
    if (vcode < 0 || vcode >= MAX_VCODES) {
        return empty;
    }
    return labels_[vcode];
}

std::string UIService::getLabelPadded(int vcode, int resultLength) const
{
    std::string label = getLabel(vcode);
    if (resultLength > static_cast<int>(label.size())) {
        label.insert(0, resultLength - label.size(), ' ');
    }
    return label;
}

void UIService::printHeader(const std::string& text) const
{
    // ConsoleUI::printHeader() takes no arguments, just prints the header
    consoleUI_.printHeader();
    if (!text.empty()) {
        std::cout << text << std::endl;
    }
}

void UIService::printStatus(const std::string& text) const
{
    std::cout << text << std::endl;
}

void UIService::printDebug(const std::string& text) const
{
    std::cout << text << std::endl;
}

void UIService::printOptions() const
{
    consoleUI_.printOptions();
}

void UIService::printHelp() const
{
    consoleUI_.printHelp();
}

void UIService::printKeylabels() const
{
    consoleUI_.printKeylabels();
}

} // namespace services
} // namespace capsicain
