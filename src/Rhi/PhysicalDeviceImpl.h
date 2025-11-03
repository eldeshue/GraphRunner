
#ifndef RHI_PHYSICAL_DEVICE_IMPL_H
#define RHI_PHYSICAL_DEVICE_IMPL_H

#include <optional>
#include <string_view>
#include <vector>

// NOLINTBEGIN
#include "GraphicsApiCore.h"

// NOLINTEND

namespace GraphRunner {
namespace Rhi {
    class Device;

    namespace Impl {
        class PhysicalDeviceImpl {
          private:
            // pinned
            PhysicalDeviceImpl(PhysicalDeviceImpl const&) = delete;
            PhysicalDeviceImpl& operator=(PhysicalDeviceImpl const&) = delete;
            PhysicalDeviceImpl(PhysicalDeviceImpl&&) = delete;
            PhysicalDeviceImpl& operator=(PhysicalDeviceImpl&&) = delete;

            // Instance is the factory
            friend class InstanceImpl;

            // handle
            VkPhysicalDevice _handle;

            PhysicalDeviceImpl( );

          public:
            ~PhysicalDeviceImpl( );

            // get property
            void log_info( ) const;

            std::optional<Device>
            create_logical_device_with_single_graphic_queue(
                std::vector<std::string_view> const& ext_names
            ) const;
        };
    } // namespace Impl
} // namespace Rhi
} // namespace GraphRunner

#endif
