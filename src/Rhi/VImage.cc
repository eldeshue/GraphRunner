
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
    _aspect_flags(aspect_flags) {
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
    _aspect_flags(other._aspect_flags) {
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
    }
    return *this;
}
