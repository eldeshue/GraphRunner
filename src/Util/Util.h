
#ifndef UTILS_H
#define UTILS_H

#include <Volk/volk.h>

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
