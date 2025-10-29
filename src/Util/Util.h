
#ifndef UTILS_H
#define UTILS_H

// NOLINTBEGIN
#include "Volk/volk.h"
// NOLINTEND

#include <exception>
#include <string>

#include "Logger.h"

namespace GraphRunner {
namespace Util {

    std::string get_result_string(VkResult errorCode);

    void check(VkResult result);
} // namespace Util
} // namespace GraphRunner

#endif
