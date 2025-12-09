#pragma once

#include <array>
#include <atomic>
#include <thread>

#include "./DeletionQueue.h"
#include "./GraphicsApiCore.h"
#include "./RhiConfig.h"

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
        std::atomic_uint64_t const& _rhi_frame_index;

        // api specific
        // gpu device
        VkDevice const _rhi_device;
        VkPhysicalDevice const _rhi_phys_device;

        // properties
        // for resource managing
        VkPhysicalDeviceDescriptorIndexingProperties _desc_index_props = { };
        VkPhysicalDeviceProperties2 _pdv_props = { };

        // allocation manager
        // memory allocator, vma
        VmaAllocator const _allocator;

        // vector of timeline semaphores
        // each semaphore is shared by each rendering thread
        // staging, deletion will be synchronized based on those semaphores
        std::vector<VkSemaphore> const* _render_semaphores;

        /* ---------- dynamic resource managing ---------- */
        // deletion manager
        // per frame deletion queue
        // need mutex for each queue
        // thread for consuming data in deletion queue
        std::array<DeletionQueue, RHI_MAX_FRAMES_IN_FLIGHT> _del_queues;
        std::jthread _del_thread;

        // desc manager
        // necessary component for bindless architecture
        // mega sized descriptor array for desc_indexing
        // TBD : after transfer

        // transfer manager
        // per frame large(128MB?) mapped buffer for staging(host visible, host coherent)
        // after frame set, reset the offset
        // use gfx command queue for copy command(TBD : ownership transfer)
        // use task queue to save offset and mapped pointer
        // record all copy command before rendering starts
        // synchronization needed, no ownership transfer
        // reallocate if the staging memory is not enough
        // TBD : after deletion
        // std::array<VStagingBuffer, RHI_MAXFRAMES_IN_FLIGHT> _staging_buffers;

        // streaming manager
        // virtual tiling,
        // TBD : after RDG

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

        /* ---------- before rendering ---------- */
        // frame_num == 0 : before rendering

        // pre_load_before_render
        // statically loaded resources

        // start rendering
        // wait for preload ends
        // create deletion thread
        void start_render(std::vector<VkSemaphore> const* semaphores);

        /* ---------- while rendering ---------- */
        // frame_num > 0 : while rendering

        // end rendering
        // wait and delete deletion threads
        // after thread ends, delete all resources after gpu stops running
        void end_render( );

        /* ---------- resource factory ---------- */
        // resource creation
        // VBuffer abstract class
        // VVertexBuffer create_vertex_buffer()
        // VIndexBuffer create_index_buffer()

        // VImage abstract class
        // VTexture2D create_texture2D();
    };
} // namespace Rhi
} // namespace GraphRunner
