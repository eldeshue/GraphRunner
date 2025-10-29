
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
            VkDevice _dvc;

            DeviceImpl( );

          public:
            ~DeviceImpl( );

            // get property
        };
    } // namespace Impl
} // namespace Rhi
} // namespace GraphRunner

#endif
