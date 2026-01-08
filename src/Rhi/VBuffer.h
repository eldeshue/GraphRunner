
#pragma once

#include <string_view>

#include "./GraphicsApiCore.h"
#include "./VResource.h"

namespace GraphRunner {
namespace Rhi {
    class VResourceManager;

    class VBuffer: public VResource {
      private:
        // no default, no copy
        VBuffer( ) = delete;
        VBuffer(VBuffer const&) = delete;
        VBuffer& operator=(VBuffer const&) = delete;

      protected:
        // resource manager is the factory
        // friend class VResourceManager;
        // factory is always has to be single
        VResourceManager* _factory;

        // buffer handle
        VkBuffer _handle = VK_NULL_HANDLE;

        // vulkan related
        VmaAllocation _alloc = nullptr;
        VmaAllocationInfo _alloc_info = { };
        VmaMemoryUsage _mem_usage;

        // for BDA
        VkDeviceAddress _device_address = 0;

        // buffer info
        VkBufferCreateInfo _ci; // current queue family index is saved here

        // status
        VkPipelineStageFlags2 _stage_flag = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 _access_flag = VK_ACCESS_2_NONE;
        uint32_t _cur_queue_family_index;

      public:
        VBuffer(
            VResourceManager& source, // factory
            VkBufferCreateInfo ci,
            VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_AUTO,
            VmaAllocationCreateFlags alloc_flags = 0,
            VkMemoryPropertyFlags req_flags = 0,
            VkMemoryPropertyFlags pref_flags = 0,
            bool is_mapped = false,
            std::string_view name = ""
        );
        virtual ~VBuffer( );

        // Movable
        VBuffer(VBuffer&& other) noexcept;
        VBuffer& operator=(VBuffer&& other) noexcept;

        /* ---------  Getter --------- */
        VkBuffer handle( ) const {
            return _handle;
        }

        // for BDA, bindless
        VkDeviceAddress device_address( ) const {
            return _device_address;
        }

        void* mapped_ptr( ) const {
            return _alloc_info.pMappedData;
        }

        VkDeviceSize size( ) const {
            return _alloc_info.size;
        } // frequently used

        VkBufferUsageFlags usage( ) const {
            return _ci.usage;
        }

        VkPipelineStageFlags2 stage_flag( ) const {
            return _stage_flag;
        }

        VkAccessFlags2 access_flag( ) const {
            return _access_flag;
        }

        uint32_t cur_queue_family_index( ) const {
            return _cur_queue_family_index;
        }

        /* ---------  Setter --------- */
        void set_stage_flag(VkPipelineStageFlags2 new_stage) {
            _stage_flag = new_stage;
        }

        void set_access_flag(VkAccessFlags2 new_access) {
            _access_flag = new_access;
        }

        void set_queue_family_index(uint32_t new_index) {
            _cur_queue_family_index = new_index;
        }

        /* ---------  IO --------- */
        // write to the buffer
        void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void write_back(
            void const* src,
            VkDeviceSize write_size,
            VkDeviceSize dst_offset = 0
        );
        // before read from the buffer
        void
        invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void read_back(
            void* dst,
            VkDeviceSize read_size,
            VkDeviceSize src_offset = 0
        );
    };
} // namespace Rhi
} // namespace GraphRunner
