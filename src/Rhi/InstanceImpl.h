
#ifndef INSTANCE_IMPL_H
#define INSTANCE_IMPL_H

// NOLINTBEGIN
#include "Volk/volk.h"
// NOLINTEND

#include <string_view>
#include <vector>

namespace GraphRunner {
namespace Rhi {
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
            using InstanceImplHandle = VkInstance;
            InstanceImplHandle _instance;

#ifdef ENABLE_VULKAN_VALIDATION
            using DebugHandle = VkDebugUtilsMessengerEXT;
            DebugHandle _dbg_messenger;
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
        };
    } // namespace Impl
} // namespace Rhi
} // namespace GraphRunner

#endif
