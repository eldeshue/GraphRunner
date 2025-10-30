
#ifndef INSTANCE_IMPL_H
#define INSTANCE_IMPL_H

// NOLINTBEGIN
#include "Volk/volk.h"
// NOLINTEND

#include <optional>
#include <string_view>
#include <vector>

namespace GraphRunner {
namespace Rhi {
    class PhysicalDevice;

    namespace Impl {

        // vulkan implementation
        class InstanceImpl {
          private:
            // no default, no copy, no move
            InstanceImpl( ) = delete;
            InstanceImpl(InstanceImpl const&) = delete;
            InstanceImpl& operator=(InstanceImpl const&) = delete;
            InstanceImpl(InstanceImpl&&) = delete;
            InstanceImpl& operator=(InstanceImpl&&) = delete;

            // volk load result
            static VkResult volk_init_result;

            // instance or DXGIFactory
            VkInstance _handle;

#ifdef ENABLE_VULKAN_VALIDATION
            VkDebugUtilsMessengerEXT _dbg_messenger;
#endif

          public:
            // default, profile 2024 roadmap
            InstanceImpl(
                std::string_view app_name,
                std::string_view engine_name,
                std::vector<std::string_view> const& required_ext_names,
                std::vector<std::string_view> const& required_laye_names
            );
            ~InstanceImpl( );

            // factory functions
            std::optional<GraphRunner::Rhi::PhysicalDevice>
            create_single_physical_device_with_best_vram( ) const;
        };
    } // namespace Impl
} // namespace Rhi
} // namespace GraphRunner

#endif
