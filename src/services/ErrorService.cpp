#include "services/ErrorService.h"
#include <iostream>

namespace capsicain {
namespace services {

void ErrorService::logError(const std::string& message)
{
    std::cout << std::endl << "ERROR: " << message << std::endl;
    errorLog_ += "\r\n" + message;
}

} // namespace services
} // namespace capsicain
