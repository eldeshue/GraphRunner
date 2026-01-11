
#include "./VImage.h"

#include "./VResourceManager.h"
#include "Util.h"

using namespace GraphRunner::Rhi;
using namespace GraphRunner::Util;

namespace {
// helper functions

// will be called debug only
void set_image_debug_name(VkDevice device, VkImage handle, char const* name) {
    VkDebugUtilsObjectNameInfoEXT nameInfo = {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT
    };
    nameInfo.objectType = VK_OBJECT_TYPE_IMAGE; // object type, buffer
    nameInfo.objectHandle = (uint64_t)(handle);
    nameInfo.pObjectName = name;

    // loaded by Volk, debug utils ext needed
    vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
}

} // namespace

VImage::VImage(
    VResourceManager& source, // factory
    VkImageCreateInfo img_ci,
    VkImageAspectFlags aspect_flags,
    VmaMemoryUsage mem_usage,
    VmaAllocationCreateFlags alloc_flags,
    VkMemoryPropertyFlags req_flags,
    VkMemoryPropertyFlags pref_flags,
    std::string_view name
) :
    _factory(&source),
    _cur_layout(img_ci.initialLayout),
    _ci(img_ci),
    _aspect_flags(aspect_flags),
    _cur_queue_family_index(_ci.pQueueFamilyIndices[0]) {
    // assert creation info
    // [Validation] Vulkan Spec violation
    if ( (_ci.imageType == VK_IMAGE_TYPE_3D) && (_ci.arrayLayers > 1) ) {
        // 3D texture can't be texture array
        throw_with_message(
            std::runtime_error(
                "VImage Creation Failed: 3D Image cannot have array layers > 1"
            ),
            "Error : VImage Creation Failed, 3D Image cannot have array layers > 1. Assert Failed."
        );
    }

    VmaAllocationCreateInfo alloc_ci = { };
    alloc_ci.usage = mem_usage;
    alloc_ci.flags = alloc_flags;
    alloc_ci.priority = 1.0f;
    // detailed control
    alloc_ci.requiredFlags = req_flags; // ex) host visible
    alloc_ci.preferredFlags = pref_flags; // ex) host coherent, host cached

    check(vmaCreateImage(
        _factory->vma_allocator( ),
        &img_ci,
        &alloc_ci,
        &_handle,
        &_alloc,
        &_alloc_info
    ));

// for debugging, set name of the object
#ifdef ENABLE_VULKAN_VALIDATION
    set_image_debug_name(_factory->device( ), _handle, name.data( ));
#endif
}

VImage::~VImage( ) {
    // push resources to the deletion queue
    // allocation and buffer
    if ( _alloc != nullptr ) {
        _factory->get_current_deletion_queue( ).enque(
            std::make_pair(_alloc, _handle)
        );
    }
}

VImage::VImage(VImage&& other) noexcept :
    _factory(other._factory),
    _handle(other._handle),
    _alloc(other._alloc),
    _alloc_info(other._alloc_info),
    _cur_layout(other._cur_layout),
    _ci(other._ci),
    _aspect_flags(other._aspect_flags),
    _cur_queue_family_index(other._cur_queue_family_index) {
    other._alloc = nullptr;
}

VImage& VImage::operator=(VImage&& other) noexcept {
    if ( this != &other ) {
        // swap
        std::swap(_factory, other._factory);
        std::swap(_handle, other._handle);
        std::swap(_alloc, other._alloc);
        std::swap(_alloc_info, other._alloc_info);
        std::swap(_cur_layout, other._cur_layout);
        std::swap(_ci, other._ci);
        std::swap(_aspect_flags, other._aspect_flags);
        std::swap(_cur_queue_family_index, other._cur_queue_family_index);
    }
    return *this;
}

// guess default layout based on image usage in creation info
VkImageLayout VImage::guess_default_layout( ) const {
    // 1. Texture for Sampling as a Shader Resource
    if ( _ci.usage & VK_IMAGE_USAGE_SAMPLED_BIT ) {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    // 2. Storage Image(UAV)
    if ( _ci.usage & VK_IMAGE_USAGE_STORAGE_BIT ) {
        return VK_IMAGE_LAYOUT_GENERAL;
    }

    // 3. Depth/Stencil
    if ( _ci.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }

    // default case
    return VK_IMAGE_LAYOUT_GENERAL;
}

VkImageViewType VImage::guess_view_type( ) const {
    switch ( _ci.imageType ) {
        case VK_IMAGE_TYPE_1D:
            return is_array( ) ? VK_IMAGE_VIEW_TYPE_1D_ARRAY
                               : VK_IMAGE_VIEW_TYPE_1D;
        case VK_IMAGE_TYPE_2D:
            return is_array( ) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                               : VK_IMAGE_VIEW_TYPE_2D;
        case VK_IMAGE_TYPE_3D:
            return VK_IMAGE_VIEW_TYPE_3D; // 3D Array is illegal
        default:
            return VK_IMAGE_VIEW_TYPE_2D; // probably unreachable
    }
}

// get memory size info from format
// AI generated
inline VImage::FormatTrait VImage::get_format_trait(VkFormat format) {
    switch ( format ) {
        // [Case 1] normal 1byte channel (R8, RG8, RGB8, RGBA8)
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8_UINT:
            return {1, 1, 1};

        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SRGB:
            return {1, 1, 2};

        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM: // for Swapchain
        case VK_FORMAT_B8G8R8A8_SRGB:
            return {1, 1, 4};

        // [Case 2] Floating Point / HDR (16F, 32F)
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return {1, 1, 8}; // 64bit
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return {1, 1, 16}; // 128bit

        // [Case 3] Depth / Stencil
        case VK_FORMAT_D32_SFLOAT:
            return {1, 1, 4};
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return {1, 1, 4}; // normally, 32bit packed

        // [Case 4] Block Compression (BCn) - compressed texture
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
            return {4, 4, 8}; // 4x4 block, 8byte

        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK: // DXT5
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
            return {4, 4, 16}; // 4x4 block, 16byte

        default:
            throw_with_message(
                std::runtime_error("Unsupported format size calculation"),
                "Error : Unsupported format size calculation"
            );
    }
}

// calculate copy region size and offset from format and extent
// AI generated, 검증 필요함
inline VImage::ImageCopyInfo VImage::get_image_copy_requirement(
    VkFormat format,
    VkExtent3D extent,
    uint32_t layer_count = 1
) {
    FormatTrait trait = get_format_trait(format);

    // 1. 가로/세로 블록 개수 계산 (반올림 처리)
    // 일반 포맷이면 (width + 0) / 1 -> width 그대로
    // BC 포맷이면 (width + 3) / 4 -> 4의 배수로 올림
    uint32_t width_in_blocks =
        (extent.width + trait.block_width - 1) / trait.block_width;
    uint32_t height_in_blocks =
        (extent.height + trait.block_height - 1) / trait.block_height;

    // 2. 깊이는 블록 개념이 없음 (3D 텍스처도 z축은 픽셀 단위)
    uint32_t depth = extent.depth;

    // 3. 전체 크기 계산
    VkDeviceSize row_size = width_in_blocks * trait.bytes_per_block;
    VkDeviceSize slice_size = row_size * height_in_blocks;
    VkDeviceSize total_size = slice_size * depth * layer_count;

    // 4. Alignment 결정
    // vkCmdCopyBufferToImage의 bufferOffset 기본 요구사항은 4바이트 정렬입니다.
    // 하지만, BCn 포맷 등 특수한 경우 16바이트(블록 크기) 정렬이 안전할 수 있습니다.
    // GPU 성능을 위해 일반적으로 16 또는 DeviceLimit의 optimalBufferCopyOffsetAlignment를 씁니다.
    // 여기서는 안전하게 16 (최대 블록 크기) 또는 4 (최소 요구사항) 중 큰 값을 씁니다.
    // TBD : alignment 문제는 굉장히 미묘하게 발생할 수 있음. 검증 필요함.
    VkDeviceSize alignment = std::max((uint32_t)4, trait.bytes_per_block);

    return {total_size, alignment, {width_in_blocks, height_in_blocks, depth}};
}
