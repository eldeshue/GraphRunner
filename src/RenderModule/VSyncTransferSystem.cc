
#include "./VSyncTransferSystem.h"

#include "./RhiConfig.h"
#include "./VBuffer.h"
#include "./VImage.h"
#include "./VResourceManager.h"
#include "./VRhi.h"
#include "./VStagingHeap.h"

using namespace GraphRunner::Rhi;

namespace GraphRunner::RenderModule {

// default staging heap size, 32MB
namespace {
    constexpr VkDeviceSize DEFAULT_STAGING_HEAP_SIZE =
        static_cast<VkDeviceSize>(RHI_DEFAULT_STAGING_HEAP_SIZE_MB) * 1024
        * 1024;
} // namespace

VSyncTransferSystem::VSyncTransferSystem(VRhi* prhi) {
    // push per frame resources
    for ( int i = 0; i < RHI_MAX_FRAMES_IN_FLIGHT; ++i ) {
        {
            // staging buffers per frame
            _staging_heaps.push_back(VStagingHeap(
                prhi->resource_manager( ),
                prhi->resource_manager( )
                    .get_pdv_props( )
                    .properties.limits.nonCoherentAtomSize,
                DEFAULT_STAGING_HEAP_SIZE // 32MB
            ));
            // queue for sync transfer per frame
            _sync_transfer_queues.push_back(SyncTransferQueue { });
        }
    }
}

VSyncTransferSystem::VSyncTransferSystem(VSyncTransferSystem&& other) noexcept :
    _before_flush_buf_barriers(std::move(other._before_flush_buf_barriers)),
    _before_flush_img_barriers(std::move(other._before_flush_img_barriers)),
    _after_flush_buf_barriers(std::move(other._after_flush_buf_barriers)),
    _after_flush_img_barriers(std::move(other._after_flush_img_barriers)),
    _staging_heaps(std::move(other._staging_heaps)),
    _sync_transfer_queues(std::move(other._sync_transfer_queues)) {}

VSyncTransferSystem& VSyncTransferSystem::operator=(VSyncTransferSystem&& other
) noexcept {
    // move
    // default impl would work maybe?
    _before_flush_buf_barriers = (std::move(other._before_flush_buf_barriers));
    _before_flush_img_barriers = (std::move(other._before_flush_img_barriers));
    _after_flush_buf_barriers = (std::move(other._after_flush_buf_barriers));
    _after_flush_img_barriers = (std::move(other._after_flush_img_barriers));
    _staging_heaps = (std::move(other._staging_heaps));
    _sync_transfer_queues = (std::move(other._sync_transfer_queues));
}

/* ---------- RenderSystem Interface --------- */
// must be called per frame,
// before dispatching render commands from game thread
void VSyncTransferSystem::reset(uint32_t cur_frame_index) {
    // reset sync transfer staging heap
    _staging_heaps[cur_frame_index].reset_offset( );

    // clear cur queue
    SyncTransferQueue& cur_queue = _sync_transfer_queues[cur_frame_index];
    cur_queue._buf_target.clear( );
    cur_queue._buf_copy_rgns.clear( );
    cur_queue._buf_cpy_infos.clear( );
    cur_queue._img_target.clear( );
    cur_queue._img_copy_rgns.clear( );
    cur_queue._img_cpy_infos.clear( );

    // TBD : get bindless update request from VResource Manager
    // bindless descriptor update must be handled by sync transfer
    // 반드시 여기서 bindless descriptor buffer의 업데이트 요청을 처리해줘야 함.
}

void VSyncTransferSystem::execute(uint32_t cur_frame_index, void* param) {
    VSyncTransferInfo& cur_info = *reinterpret_cast<VSyncTransferInfo*>(param);

    // copy to the cur staging buffer
    VStagingHeap& cur_buffer = _staging_heaps[cur_frame_index];
    auto result =
        cur_buffer.try_push(cur_info.psrc, cur_info.size, cur_info.alignment);
    if ( !result.has_value( ) ) {
        return;
    }

    // accumulate transfer request
    SyncTransferQueue& cur_queue = _sync_transfer_queues[cur_frame_index];
    std::visit(
        [&](auto&& target) {
            using T = std::decay_t<decltype(target)>;

            if constexpr ( std::is_same_v<T, VSyncBufferTransferInfo> ) {
                // target is VBuffer
                cur_queue._buf_target.insert(target.dst);

                VkBufferCopy2 cpy_rgn { };
                cpy_rgn.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
                cpy_rgn.pNext = nullptr;
                cpy_rgn.size = result->size;
                cpy_rgn.srcOffset = result->offset;
                cpy_rgn.dstOffset = target.dst_offset;
                cur_queue._buf_copy_rgns.push_back(cpy_rgn);

                VkCopyBufferInfo2 cpy_info { };
                cpy_info.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
                cpy_info.pNext = nullptr;
                cpy_info.srcBuffer = cur_buffer.handle( ).handle( );
                cpy_info.dstBuffer = target.dst->handle( );
                cpy_info.regionCount = 1;
                cpy_info.pRegions = &cur_queue._buf_copy_rgns.back(
                ); // deque does not reallocate, no pointer invalidation
                cur_queue._buf_cpy_infos.push_back(cpy_info);
            } else if constexpr ( std::is_same_v<T, VSyncImageTransferInfo> ) {
                // target is VImage
                cur_queue._img_target.insert(target.dst);

                VkBufferImageCopy2 cpy_rgn { };
                cpy_rgn.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
                cpy_rgn.pNext = nullptr;
                cpy_rgn.bufferOffset = result->offset;
                cpy_rgn.bufferRowLength = 0; // tightly packed
                cpy_rgn.bufferImageHeight = 0; // tightly packed
                cpy_rgn.imageExtent = target.dst_range;
                cpy_rgn.imageOffset = target.dst_pos;
                cpy_rgn.imageSubresource = target.dst_area;
                cur_queue._img_copy_rgns.push_back(cpy_rgn);

                VkCopyBufferToImageInfo2 cpy_info { };
                cpy_info.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
                cpy_info.pNext = nullptr;
                cpy_info.srcBuffer = cur_buffer.handle( ).handle( );
                cpy_info.dstImage = target.dst->handle( );
                cpy_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                cpy_info.regionCount = 1;
                cpy_info.pRegions = &cur_queue._img_copy_rgns.back( );
                cur_queue._img_cpy_infos.push_back(cpy_info);
            } else {
                // unreachable
            }
        },
        cur_info.target_info
    );
}

/* ---------- helper functions ---------- */
// Warning : transfering same data with different logic is logical error
// must be handled by developer
// TBD : building COPY_PASS
// only for simple transfer, more complex usage needs building COPY_PASS in RDG
void VSyncTransferSystem::flush(
    VkCommandBuffer cmd_buffer,
    uint32_t cur_frame_index
) {
    SyncTransferQueue& cur_queue = _sync_transfer_queues[cur_frame_index];
    VStagingHeap& cur_buffer = _staging_heaps[cur_frame_index];

    // ----------------------------------------------------------------
    // 1. Reset Barrier buffers
    // ----------------------------------------------------------------
    _before_flush_buf_barriers.clear( );
    _before_flush_img_barriers.clear( );
    _after_flush_buf_barriers.clear( );
    _after_flush_img_barriers.clear( );

    // ----------------------------------------------------------------
    // 2. Prepare Barriers
    // ----------------------------------------------------------------
    {
        // fill buffer barriers
        VkBufferMemoryBarrier2 buf_bar = { };
        buf_bar.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        buf_bar.pNext = nullptr;
        buf_bar.offset = 0;
        buf_bar.size = VK_WHOLE_SIZE;
        buf_bar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        buf_bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        // reserve buffers
        _before_flush_buf_barriers.reserve(
            _before_flush_buf_barriers.size( ) + cur_queue._buf_target.size( )
        );
        _after_flush_buf_barriers.reserve(
            _after_flush_buf_barriers.size( ) + cur_queue._buf_target.size( )
        );
        for ( VBuffer* target : cur_queue._buf_target ) {
            buf_bar.buffer = target->handle( );
            // to copy destination, for now
            buf_bar.srcStageMask = target->stage_flag( );
            buf_bar.srcAccessMask = target->access_flag( );
            buf_bar.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
            buf_bar.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            _before_flush_buf_barriers.push_back(buf_bar);
            // to shader read, for later
            buf_bar.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
            buf_bar.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            buf_bar.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            buf_bar.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
            _after_flush_buf_barriers.push_back(buf_bar);

            // set hand-over status
            // default status after transfer
            target->set_stage_flag(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
            target->set_access_flag(VK_ACCESS_2_MEMORY_READ_BIT);
        }

        // fill image barriers
        VkImageMemoryBarrier2 img_bar = { };
        img_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        img_bar.pNext = nullptr;
        // no ownership transfer, keep same queue family index
        img_bar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        img_bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // whole range
        img_bar.subresourceRange.baseArrayLayer = 0;
        img_bar.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        img_bar.subresourceRange.baseMipLevel = 0;
        img_bar.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

        // reserve buffers
        _before_flush_img_barriers.reserve(
            _before_flush_img_barriers.size( ) + cur_queue._img_target.size( )
        );
        _after_flush_img_barriers.reserve(
            _after_flush_img_barriers.size( ) + cur_queue._img_target.size( )
        );

        for ( VImage* target : cur_queue._img_target ) {
            img_bar.image = target->handle( );
            img_bar.subresourceRange.aspectMask = target->aspect_flags( );

            // to copy destination, for now
            img_bar.oldLayout = target->cur_layout( );
            img_bar.srcStageMask = target->stage_flag( );
            img_bar.srcAccessMask = target->access_flag( );
            img_bar.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            img_bar.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
            img_bar.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            _before_flush_img_barriers.push_back(img_bar);
            // to shared read, for later
            img_bar.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            img_bar.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
            img_bar.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            img_bar.newLayout = target->guess_default_layout( );
            img_bar.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            img_bar.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
            _after_flush_img_barriers.push_back(img_bar);

            // set hand-over status
            // default status after transfer
            target->set_layout(target->guess_default_layout( ));
            target->set_stage_flag(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
            target->set_access_flag(VK_ACCESS_2_MEMORY_READ_BIT);
        }
    }

    // ----------------------------------------------------------------
    // 3. Submit Pre-Barriers (Wait for previous usage -> Ready to Transfer)
    // ----------------------------------------------------------------
    VkDependencyInfo dp_info = { };
    dp_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dp_info.pNext = nullptr;
    dp_info.dependencyFlags = 0; // out of render pass, no region control
    dp_info.memoryBarrierCount = 0;
    dp_info.pMemoryBarriers = nullptr;
    if ( !_before_flush_buf_barriers.empty( )
         || !_before_flush_img_barriers.size( ) ) {
        dp_info.bufferMemoryBarrierCount = _before_flush_buf_barriers.size( );
        dp_info.pBufferMemoryBarriers = _before_flush_buf_barriers.data( );
        dp_info.imageMemoryBarrierCount = _before_flush_img_barriers.size( );
        dp_info.pImageMemoryBarriers = _before_flush_img_barriers.data( );
        vkCmdPipelineBarrier2(cmd_buffer, &dp_info);
    }

    // ----------------------------------------------------------------
    // 4. Record Copy Commands
    // ----------------------------------------------------------------
    // flush staging heap for visibility
    cur_buffer.flush_staging( );

    // recording copy start
    {
        // iterate with index, for pointer safety
        for ( size_t i = 0; i < cur_queue._buf_cpy_infos.size( ); ++i ) {
            // record copy
            vkCmdCopyBuffer2(cmd_buffer, &cur_queue._buf_cpy_infos[i]);
        }
        // clear buffer
        cur_queue._buf_target.clear( );
        cur_queue._buf_cpy_infos.clear( );
        cur_queue._buf_copy_rgns.clear( );
    }
    {
        // iterate with index, for pointer safety
        for ( size_t i = 0; i < cur_queue._img_cpy_infos.size( ); ++i ) {
            // record copy
            vkCmdCopyBufferToImage2(cmd_buffer, &cur_queue._img_cpy_infos[i]);
        }
        cur_queue._img_target.clear( );
        cur_queue._img_cpy_infos.clear( );
        cur_queue._img_copy_rgns.clear( );
    }

    // ----------------------------------------------------------------
    // 5. Submit Post-Barriers (Wait for transfer -> Ready to Read)
    // ----------------------------------------------------------------
    // after flush pipeline barrier
    // won't be needed for RDG, this is why this function is not for RDG
    if ( !_after_flush_buf_barriers.empty( )
         || !_after_flush_img_barriers.empty( ) ) {
        dp_info.bufferMemoryBarrierCount = _after_flush_buf_barriers.size( );
        dp_info.pBufferMemoryBarriers = _after_flush_buf_barriers.data( );
        dp_info.imageMemoryBarrierCount = _after_flush_img_barriers.size( );
        dp_info.pImageMemoryBarriers = _after_flush_img_barriers.data( );
        vkCmdPipelineBarrier2(cmd_buffer, &dp_info);
    }
}

} // namespace GraphRunner::RenderModule
