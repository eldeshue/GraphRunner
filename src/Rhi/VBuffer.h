
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
        VResourceManager& _factory;

        // vulkan related
        VmaAllocation _alloc = nullptr;
        VkBuffer _handle = VK_NULL_HANDLE;
        VkDeviceAddress _device_address = 0;
        VmaAllocationInfo _alloc_info = { };

        // flags
        // vulkan memory type flags
        VkBufferUsageFlags _buffer_usage_flags = 0;
        // vma usage
        VmaMemoryUsage _mem_usage;

      public:
        VBuffer(
            VResourceManager& source, // factory
            VkDeviceSize size, // required size
            VkBufferUsageFlags buffer_usage_flags,
            VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_AUTO,
            VmaAllocationCreateFlags alloc_flags = 0,
            VkMemoryPropertyFlags req_flags = 0,
            VkMemoryPropertyFlags pref_flags = 0,
            bool is_mapped = false,
            std::string_view name = ""
        );
        virtual ~VBuffer( );

        // Movable
        VBuffer(VBuffer&& other);
        VBuffer& operator=(VBuffer&& other);

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
            return _buffer_usage_flags;
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
