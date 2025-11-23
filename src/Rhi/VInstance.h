#pragma once

// NOLINTBEGIN
#include "GraphicsApiCore.h"
// NOLINTEND

#include <optional>
#include <string_view>
#include <vector>

namespace GraphRunner {
namespace Rhi {
    class VPhysicalDevice;

    // vulkan implementation
    class VInstance {
      private:
        // no copy
        VInstance(VInstance const&) = delete;
        VInstance& operator=(VInstance const&) = delete;

        // volk load result
        static VkResult volk_init_result;

        // instance or DXGIFactory
        VkInstance _handle;

        VkDebugUtilsMessengerEXT _dbg_messenger;

      public:
        // default construct, empty
        VInstance( );
        // default profile is roadmap2024
        VInstance(
            std::string_view app_name,
            std::string_view engine_name,
            std::vector<std::string_view> const& required_ext_names,
            std::vector<std::string_view> const& required_laye_names
        );
        VInstance(VInstance&& other) noexcept;
        VInstance& operator=(VInstance&& other) noexcept;
        ~VInstance( );

        VkInstance handle( ) const;

        // factory functions
        std::optional<GraphRunner::Rhi::VPhysicalDevice>
        create_single_physical_device_with_best_vram( ) const;
    };
} // namespace Rhi
} // namespace GraphRunner
