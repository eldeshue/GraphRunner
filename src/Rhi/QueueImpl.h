
#ifndef RHI_QUEUE_IMPL_H
#define RHI_QUEUE_IMPL_H

// NOLINTBEGIN
#include "Volk/volk.h"

// NOLINTEND

namespace GraphRunner {
namespace Rhi {
    namespace Impl {

        class DeviceImpl;

        class QueueImpl {
          private:
            QueueImpl(QueueImpl const&) = delete;
            QueueImpl& operator=(QueueImpl const&) = delete;

            friend class DeviceImpl;

            VkQueue _handle;
            VkQueueFamilyProperties* _info;

            // Device is the factory
            QueueImpl( );

          public:
            ~QueueImpl( );

            // submit command queue
        };
    } // namespace Impl
} // namespace Rhi
} // namespace GraphRunner

#endif
