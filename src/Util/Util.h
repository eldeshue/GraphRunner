
#ifndef UTILS_H
#define UTILS_H

// NOLINTBEGIN
#include "./GraphicsApiCore.h"
// NOLINTEND

#include <exception>
#include <string>

#include "Logger.h"

namespace GraphRunner {
namespace Util {

    std::string get_result_string(VkResult errorCode);

    void check(VkResult result);

    void set_vp_capabilities(VpCapabilities& cap);

    constexpr VkDeviceSize
    cal_paded_size(VkDeviceSize data_size, VkDeviceSize alignment);
} // namespace Util
} // namespace GraphRunner

#endif
