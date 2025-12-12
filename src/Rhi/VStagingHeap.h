
#pragma once

#include <atomic>
#include <optional>

#include "./GraphicsApiCore.h"
#include "./VBuffer.h"
#include "./VResourceManager.h"

namespace GraphRunner {
namespace Rhi {
    struct StagedData {
        VkBuffer buffer;
        VkDeviceSize offset;
        VkDeviceSize size;
    };

    class VStagingHeap {
      private:
        // no default, no copy
        VStagingHeap( ) = delete;
        VStagingHeap(VStagingHeap const&) = delete;
        VStagingHeap& operator=(VStagingHeap const&) = delete;

        // current offset
        std::atomic_uint64_t _cur_offset = 0;
        VkDeviceSize _non_coherent_alignment;
        VBuffer _buffer;

      public:
        VStagingHeap(
            VResourceManager& source,
            VkDeviceSize non_coherent_alignment,
            VkDeviceSize size
        );
        ~VStagingHeap( );

        // move
        VStagingHeap(VStagingHeap&& other) noexcept;
        VStagingHeap& operator=(VStagingHeap&& other) noexcept;

        std::optional<StagedData>
        try_push(void* src, VkDeviceSize size, VkDeviceSize alignment);

        // reset offset
        // must be called by render thread, after flushing
        void reset_offset( ) noexcept {
            _cur_offset.store(0);
        }

        // flush buffer before from 0 to offset
        // offset must always be aligned
        // must be called by render thread, before recording copy
        void flush_staging( ) {
            _buffer.flush(_cur_offset.load( ), 0);
        }
    };
} // namespace Rhi
} // namespace GraphRunner
