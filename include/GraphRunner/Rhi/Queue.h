
#ifndef RHI_QUEUE
#define RHI_QUEUE

namespace GraphRunner {
namespace Rhi {
    namespace Impl {
        class QueueImpl;
        class DeviceImpl;
    } // namespace Impl

    class Queue {
      private:
        Queue(Queue const&) = delete;
        Queue& operator=(Queue const&) = delete;

        Impl::QueueImpl* _impl;

        friend class Impl::DeviceImpl;

        // Device is the factory
        Queue( );

      public:
        ~Queue( );
        Queue(Queue&& other) noexcept;
        Queue& operator=(Queue&& other) noexcept;

        // submit command queue
    };
} // namespace Rhi
} // namespace GraphRunner

#endif
