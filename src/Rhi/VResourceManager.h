#pragma once

#include <atomic>

#include "./GraphicsApiCore.h"

namespace GraphRunner {
namespace Rhi {
    class VResourceManager {
      private:
        // no default, no clone, no move
        VResourceManager( );
        VResourceManager(VResourceManager const&) = delete;
        VResourceManager& operator=(VResourceManager const&) = delete;
        VResourceManager(VResourceManager&&) = delete;
        VResourceManager& operator=(VResourceManager&&) = delete;

        // frame index of Rhi
        // for timeline semaphore
        std::atomic_uint64_t& _rhi_frame_index;

        // api specific
        // gpu device
        VkDevice _rhi_device;

        // allocation manager
        // memory allocator, vma
        VmaAllocator _allocator;

        // static resource managing
        // do not call during the level
        // only delete after the scene ended

        // desc manager
        // necessary component for bindless architecture
        // large descriptor array for desc_indexing

        // deletion manager
        // per frame deletion queue
        // need mutex for each queue
        // thread for consuming data in deletion queue

        // transfer manager
        // per frame large(256MB?) mapped buffer for staging(host visible, host coherent)
        // after frame set, reset the offset
        // use gfx command queue for copy command
        // use task queue to save offset and mapped pointer
        // record all copy command before rendering starts
        // synchronization needed, no ownership transfer

        // need fall-back logic,

      public:
        // basic creation method
        // need reference of flight frame number, ref to frame index, vk device
        // maybe another queue for transfer
        VResourceManager(
            std::atomic_uint64_t& frame_idx,
            VkInstance inst,
            VkPhysicalDevice pdvc,
            VkDevice dvc
        );
        ~VResourceManager( );

        // factory functions
        // create resources
        // ex) VTexture2D CreateTexture2D() const;
    };
} // namespace Rhi
} // namespace GraphRunner
