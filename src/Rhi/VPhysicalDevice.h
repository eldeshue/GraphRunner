
#pragma once

#include <optional>
#include <string_view>
#include <vector>

// NOLINTBEGIN
#include "GraphicsApiCore.h"

// NOLINTEND

namespace GraphRunner {
namespace Rhi {
    class VDevice;

    class VPhysicalDevice {
      private:
        VPhysicalDevice(VPhysicalDevice const&) = delete;
        VPhysicalDevice& operator=(VPhysicalDevice const&) = delete;

        // Instance is the factory
        friend class VInstance;

        // handle
        VkPhysicalDevice _handle;

      public:
        VPhysicalDevice( );
        VPhysicalDevice(VPhysicalDevice&& other) noexcept;
        VPhysicalDevice& operator=(VPhysicalDevice&& other) noexcept;
        ~VPhysicalDevice( );

        VkPhysicalDevice handle( ) const;

        // get property
        void log_info( ) const;

        std::optional<VDevice> create_logical_device_with_single_graphic_queue(
            std::vector<std::string_view> const& ext_names
        ) const;
        VDevice create_logical_device_with_all_queues(
            std::vector<std::string_view> const& ext_names
        ) const;
    };
} // namespace Rhi
} // namespace GraphRunner
