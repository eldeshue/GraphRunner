
#pragma once

#include <string_view>

#include "./GraphicsApiCore.h"
#include "./VResource.h"

namespace GraphRunner {
namespace Rhi {
    class VResourceManager;

    class VImage: public VResource {
      private:
        // no default, no copy
        VImage( ) = delete;
        VImage(VImage const&) = delete;
        VImage& operator=(VImage const&) = delete;

      protected:
        // resource manager is the factory
        // friend class VResourceManager;
        // factory is always has to be single
        VResourceManager* _factory;

        // vulkan image handle
        VkImage _handle = VK_NULL_HANDLE;

        // vma related
        VmaAllocation _alloc = nullptr;
        VmaAllocationInfo _alloc_info = { };

        // vulkan related status
        VkImageLayout _cur_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageCreateInfo _ci;
        VkImageAspectFlags _aspect_flags;

        VkPipelineStageFlags2 _stage_flag = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 _access_flag = VK_ACCESS_2_NONE;

      public:
        // basic constructr
        VImage(
            VResourceManager& source, // factory
            VkImageCreateInfo ci,
            VkImageAspectFlags aspect_flags,
            VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_AUTO,
            VmaAllocationCreateFlags alloc_flags = 0,
            VkMemoryPropertyFlags req_flags = 0,
            VkMemoryPropertyFlags pref_flags = 0,
            std::string_view name = ""
        );
        // destructor
        virtual ~VImage( );
        // move
        VImage(VImage&& other) noexcept;
        VImage& operator=(VImage&& other) noexcept;

        /* ---------  Getter --------- */
        // vulkan image handle
        VkImage handle( ) const {
            return _handle;
        }

        VResourceManager* factory( ) const {
            return _factory;
        }

        // vulkan related status
        VkImageLayout cur_layout( ) const {
            return _cur_layout;
        }

        VkFormat format( ) const {
            return _ci.format;
        }

        VkImageUsageFlags usage_flags( ) const {
            return _ci.usage;
        }

        VkImageAspectFlags aspect_flags( ) const {
            return _aspect_flags;
        }

        // other meta data
        VkExtent3D extent( ) const {
            return _ci.extent;
        }

        uint32_t mip_level_cnt( ) const {
            return _ci.mipLevels;
        }

        uint32_t array_layer_cnt( ) const {
            return _ci.arrayLayers;
        }

        size_t size( ) const {
            return _alloc_info.size;
        }

        VkPipelineStageFlags2 stage_flag( ) const {
            return _stage_flag;
        }

        VkAccessFlags2 access_flag( ) const {
            return _access_flag;
        }

        /* ---------  Setter --------- */
        // set current layout status
        // actual layout transfer needs command recording
        void set_layout(VkImageLayout new_layout) {
            _cur_layout = new_layout;
        }

        void set_stage_flag(VkPipelineStageFlags2 new_stage) {
            _stage_flag = new_stage;
        }

        void set_access_flag(VkAccessFlags2 new_access) {
            _access_flag = new_access;
        }
    };
} // namespace Rhi
} // namespace GraphRunner
