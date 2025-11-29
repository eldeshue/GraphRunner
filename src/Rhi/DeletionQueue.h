
#pragma once

#include <mutex>
#include <variant>
#include <vector>

#include "./GraphicsApiCore.h"

namespace GraphRunner {
namespace Rhi {
    class DeletionQueue {
      public:
        using VmaImage = std::pair<VmaAllocation, VkImage>;
        using VmaBuffer = std::pair<VmaAllocation, VkBuffer>;

        // GPU 동기화가 필요한 삭제 대상
        // 필요에 따라서 확장 가능
        // 확장 시 반드시 process_deletion에 visit pattern 추가
        using DeletionObj = std::variant<
            VmaImage, // ex) texture
            VmaBuffer, // ex) index buffer
            VkImageView, // ex) view of image, dereferenced by pipeline
            VkSampler, // sampler, with texture
            VkFramebuffer // frame buffer, for resizing
            >;

      private:
        // no copy
        DeletionQueue(DeletionQueue const&) = delete;
        DeletionQueue& operator=(DeletionQueue const&) = delete;

        // fields, owning
        std::vector<DeletionObj> _queue; // for enque
        std::vector<DeletionObj> _local_deletion_queue; // for swap
        std::mutex _mtx;

        // logical device, not owning
        VkDevice _device;
        // vma allocator, not owning
        VmaAllocator _allocator;

      public:
        // default creation, move required
        DeletionQueue( );
        DeletionQueue(VkDevice device, VmaAllocator allocator);
        ~DeletionQueue( );
        DeletionQueue(DeletionQueue&& other) noexcept;
        DeletionQueue& operator=(DeletionQueue&& other) noexcept;

        void enque(DeletionObj obj);
        void deque( );
        void clear_all( );
    };
} // namespace Rhi
} // namespace GraphRunner
