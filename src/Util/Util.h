
#ifndef UTILS_H
#define UTILS_H

// NOLINTBEGIN
#include "GraphicsApiCore.h"
// NOLINTEND

#include <exception>
#include <string>

#include "Logger.h"

namespace GraphRunner {
namespace Util {

    std::string get_result_string(VkResult errorCode);

    void check(VkResult result);

    void set_vp_capabilities(VpCapabilities& cap);
} // namespace Util
} // namespace GraphRunner

#endif
