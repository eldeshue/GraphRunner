
#ifndef RHI_DEVICE_IMPL_H
#define RHI_DEVICE_IMPL_H

// NOLINTBEGIN
#include "Volk/volk.h"

// NOLINTEND

namespace GraphRunner {
namespace Rhi {
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

            // queue data
            uint32_t graphic_queue_limit;
            uint32_t graphic_queue_cnt;
            uint32_t compute_queue_limit;
            uint32_t compute_queue_cnt;
            uint32_t transfer_queue_limit;
            uint32_t transfer_queue_cnt;

            DeviceImpl( );

          public:
            ~DeviceImpl( );

            // get property
            uint32_t get_graphic_queue_limit( ) const;
            uint32_t get_graphic_queue_cnt( ) const;
            uint32_t get_compute_queue_limit( ) const;
            uint32_t get_compute_queue_cnt( ) const;
            uint32_t get_transfer_queue_limit( ) const;
            uint32_t get_transfer_queue_cnt( ) const;
        };
    } // namespace Impl
} // namespace Rhi
} // namespace GraphRunner

#endif
