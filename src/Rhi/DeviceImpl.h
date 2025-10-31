
#ifndef RHI_DEVICE_IMPL_H
#define RHI_DEVICE_IMPL_H

// NOLINTBEGIN
#include "Volk/volk.h"

// NOLINTEND

#include <optional>
#include <tuple>
#include <vector>

namespace GraphRunner {
namespace Rhi {
    class Queue;

    namespace Impl {
        class DeviceImpl {
          private:
            // pinned
            DeviceImpl(DeviceImpl const&) = delete;
            DeviceImpl& operator=(DeviceImpl const&) = delete;
            DeviceImpl(DeviceImpl&&) = delete;
            DeviceImpl& operator=(DeviceImpl&&) = delete;

            // Physicla device is the factory
            friend class PhysicalDeviceImpl;

            // handle
            VkDevice _handle;

            // queu info
            // queue family properties, queue creation info, created queue count
            using QueuInfo = std::tuple<
                VkQueueFamilyProperties,
                VkDeviceQueueCreateInfo,
                uint32_t>;
            std::vector<QueuInfo> _queue_infos;

            DeviceImpl( );

          public:
            ~DeviceImpl( );

            // create queue
            std::optional<Queue> create_queue_with_flags(VkQueueFlags flags);
            std::optional<Queue> create_graphics_queue( );
            std::optional<Queue> create_compute_queue( );
            std::optional<Queue> create_transfer_queue( );
        };
    } // namespace Impl
} // namespace Rhi
} // namespace GraphRunner

#endif
