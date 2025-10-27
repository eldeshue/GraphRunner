
#ifndef RHI_PHYSICAL_DEVICE
#define RHI_PHYSICAL_DEVICE

namespace GraphRunner {
namespace Rhi {
    namespace Impl {
        class PhysicalDeviceImpl;
    }

    class PhysicalDevice {
      private:
        PhysicalDevice(PhysicalDevice const&) = delete;
        PhysicalDevice& operator=(PhysicalDevice const&) = delete;

        Impl::PhysicalDeviceImpl* _impl;

        // instance is the factory
        friend class Impl::InstanceImpl;

        // constructors, will be called by instance
        // factory will do the rest
        PhysicalDevice( );

      public:
        ~PhysicalDevice( );
        PhysicalDevice(PhysicalDevice&& other) noexcept;
        PhysicalDevice& operator=(PhysicalDevice&& other) noexcept;

        // get property of the physical device
        void log_info( ) const;

        // create logical device
    };
} // namespace Rhi
} // namespace GraphRunner

#endif
