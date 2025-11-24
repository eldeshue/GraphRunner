#pragma once

#include <atomic>

#include "./GraphicsApiCore.h"

namespace GraphRunner {
namespace Rhi {
    class VResourceManager {
      private:
        // no default, no clone, no move
        VResourceManager( ) = delete;
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

        // vector of timeline semaphores
        // each semaphore is shared by each rendering thread
        // staging, deletion will be synchronized based on those semaphores
        std::vector<VkSemaphore>* _semaphores;

        /* ---------- dynamic resource managing ---------- */
        // deletion manager
        // per frame deletion queue
        // need mutex for each queue
        // thread for consuming data in deletion queue

        // desc manager
        // necessary component for bindless architecture
        // mega sized descriptor array for desc_indexing

        // transfer manager
        // per frame large(128MB?) mapped buffer for staging(host visible, host coherent)
        // after frame set, reset the offset
        // use gfx command queue for copy command(TBD : ownership transfer)
        // use task queue to save offset and mapped pointer
        // record all copy command before rendering starts
        // synchronization needed, no ownership transfer
        // reallocate if the staging memory is not enough

        // streaming manager
        // virtual tiling, TBD after RDG

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

        // set semaphores
        void set_semaphores(std::vector<VkSemaphore>* semaphores);

        // resource creation
        // VVertexBuffer create_vertex_buffer()
        // VIndexBuffer create_index_buffer()
        // VTexture2D create_texture2D();

        // resource deletion
        // enqueue by destruction of each objects
        // dequeue(consumption) when the frame starts
        // clear all queues, must be called after the scene ends
    };
} // namespace Rhi
} // namespace GraphRunner
