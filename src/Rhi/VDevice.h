
#pragma once

// NOLINTBEGIN
#include "GraphicsApiCore.h"
// NOLINTEND

#include <optional>
#include <tuple>
#include <vector>

namespace GraphRunner {
namespace Rhi {
    class VQueue;

    class VDevice {
      private:
        // pinned
        VDevice(VDevice const&) = delete;
        VDevice& operator=(VDevice const&) = delete;

        // Physicla device is the factory
        friend class VPhysicalDevice;

        // handle
        VkDevice _handle;

        // queu info
        // queue family properties, queue creation info, created queue count
        using QueuInfo = std::
            tuple<VkQueueFamilyProperties, VkDeviceQueueCreateInfo, uint32_t>;
        std::vector<QueuInfo> _queue_infos;

        VDevice( );

      public:
        VDevice(VDevice&& other) noexcept;
        VDevice& operator=(VDevice&& other) noexcept;
        ~VDevice( );

        // create queue
        std::optional<VQueue> create_queue_with_flags(VkQueueFlags flags);
        std::optional<VQueue> create_graphics_queue( );
        std::optional<VQueue> create_compute_queue( );
        std::optional<VQueue> create_transfer_queue( );
    };
} // namespace Rhi
} // namespace GraphRunner
