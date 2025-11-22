
#pragma once

// NOLINTBEGIN
#include "GraphicsApiCore.h"

// NOLINTEND

namespace GraphRunner {
namespace Rhi {
    class VDevice;

    class VQueue {
      private:
        VQueue(VQueue const&) = delete;
        VQueue& operator=(VQueue const&) = delete;

        friend class VDevice;

        VkQueue _handle;
        VkQueueFamilyProperties* _info;

        // Device is the factory
        VQueue( );

      public:
        VQueue(VQueue&& other) noexcept;
        VQueue& operator=(VQueue&& other) noexcept;
        ~VQueue( );

        VkQueue handle( ) const;

        // submit command queue
    };
} // namespace Rhi
} // namespace GraphRunner
