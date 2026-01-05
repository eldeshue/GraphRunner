#pragma once

#include <atomic>
#include <optional>
#include <thread>
#include <vector>

#include "./DeletionQueue.h"
#include "./GraphicsApiCore.h"
#include "./RhiConfig.h"
#include "./VStagingHeap.h"

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

        /* ---------- dynamic resource managing ---------- */
        // deletion manager
        // per frame deletion queue
        std::vector<DeletionQueue> _del_queues;

        // desc manager
        // necessary component for bindless architecture
        // mega sized descriptor array for desc_indexing
        // TBD : after transfer

        // sync transfer
        // sync transfer with gfx queue
        // per frame mapped buffer for staging(host visible, host coherent)
        // after frame set, reset the offset
        // use task queue to save offset and mapped pointer
        // record all copy command before rendering starts
        // synchronized using pipeline barrier, no ownership transfer
        std::vector<VStagingHeap> _staging_heaps;

        // async transfer
        // async transfer with transfer only queue and temporal staging buffer
        // will update desc index of resource
        // ownership transfer from transfer queue to gfx queue needed
        // reallocate if the staging memory is not enough
        // TBD : after RDG

        // streaming
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

        /* ---------- Getter ---------- */
        VkDevice device( ) const {
            return _rhi_device;
        }

        VmaAllocator vma_allocator( ) const {
            return _allocator;
        }

        DeletionQueue& get_current_deletion_queue( ) {
            // use frame index
            return _del_queues
                [_rhi_frame_index.load( ) % RHI_MAX_FRAMES_IN_FLIGHT];
        }

        /**
         * @brief stage data for simple uploading. 
         * If the uploading data is too big, need transfer-only-queue or tile-based-streaming.
         * those will be implemented later.
         * 
         * @return returns option of Staged Information, return none if the heap runs out.
         */
        std::optional<StagedData>
        try_stage_data(void* src, VkDeviceSize size, VkDeviceSize alignment) {
            return _staging_heaps
                [_rhi_frame_index.load( ) % RHI_MAX_FRAMES_IN_FLIGHT]
                    .try_push(src, size, alignment);
        }

        /* ---------- before rendering ---------- */
        // frame_num == 0 : before rendering

        // pre_load_before_render
        // statically loaded resources

        // start rendering
        // wait for preload ends
        // create deletion thread
        void start_render( );

        /* ---------- while rendering ---------- */
        // frame_num > 0 : while rendering

        // end rendering
        // wait and delete deletion threads
        // after thread ends, delete all resources after gpu stops running
        void end_render( );

        /* ---------- frame based resource control ---------- */
        // must be called by main rendering thread, owner of the resource
        void start_frame( );
        void end_frame( );

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
