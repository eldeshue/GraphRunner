
#ifndef RHI_PHYSICAL_DEVICE_IMPL_H
#define RHI_PHYSICAL_DEVICE_IMPL_H

// NOLINTBEGIN
#include "Volk/volk.h"

// NOLINTEND

namespace GraphRunner {
namespace Rhi {
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
            VkPhysicalDevice _pdvc;

          public:
            PhysicalDeviceImpl( );
            ~PhysicalDeviceImpl( );

            // get property
        };
    } // namespace Impl
} // namespace Rhi
} // namespace GraphRunner

#endif
