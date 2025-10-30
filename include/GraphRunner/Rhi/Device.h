
#ifndef RHI_DEVICE
#define RHI_DEVICE

namespace GraphRunner {
namespace Rhi {
    namespace Impl {
        class DeviceImpl;
        class PhysicalDeviceImpl;
    } // namespace Impl

    class Device {
      private:
        Device(Device const&) = delete;
        Device& operator=(Device const&) = delete;

        Impl::DeviceImpl* _impl;

        // instance is the factory
        friend class Impl::PhysicalDeviceImpl;

        // constructors, will be called by physical device
        // factory will do the rest
        Device( );

      public:
        ~Device( );
        Device(Device&& other) noexcept;
        Device& operator=(Device&& other) noexcept;

        // get property of the device
        uint32_t get_graphic_queue_limit( ) const;
        uint32_t get_graphic_queue_cnt( ) const;
        uint32_t get_compute_queue_limit( ) const;
        uint32_t get_compute_queue_cnt( ) const;
        uint32_t get_transfer_queue_limit( ) const;
        uint32_t get_transfer_queue_cnt( ) const;

        // create queue
    };
} // namespace Rhi
} // namespace GraphRunner

#endif
