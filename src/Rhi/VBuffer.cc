
#include "./VBuffer.h"

#include "Util.h"

using namespace GraphRunner::Rhi;
using namespace GraphRunner::Util;

namespace {

// will be called debug only
void set_buffer_debug_name(VkDevice device, VkBuffer handle, char const* name) {
    VkDebugUtilsObjectNameInfoEXT nameInfo = {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT
    };
    nameInfo.objectType = VK_OBJECT_TYPE_BUFFER; // object type, buffer
    nameInfo.objectHandle = (uint64_t)(handle);
    nameInfo.pObjectName = name;

    // loaded by Volk, debug utils ext needed
    vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
}

} // namespace

VBuffer::VBuffer(
    VResourceManager& source, // factory
    VkDeviceSize size, // required size
    VkBufferUsageFlags buffer_usage_flags,
    VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_AUTO, // auto preferred
    VmaAllocationCreateFlags alloc_flags = 0,
    VkMemoryPropertyFlags req_flags = 0,
    VkMemoryPropertyFlags pref_flags = 0,
    bool is_mapped = false,
    std::string_view name = ""
) :
    _factory(source),
    _buffer_usage_flags(buffer_usage_flags),
    _mem_usage(mem_usage) {
    // initialize memory mapping
    // must set one of next two bit
    // VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT for write,staging buffer
    // VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT for read back
    if ( is_mapped == true ) {
        alloc_flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    // create buffer using vma
    VkBufferCreateInfo buffer_ci = { };
    buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_ci.size = size;
    buffer_ci.usage = _buffer_usage_flags;

    VmaAllocationCreateInfo alloc_ci = { };
    alloc_ci.usage = _mem_usage; // set auto and let vma to decide
    alloc_ci.flags = alloc_flags;
    // detailed control
    alloc_ci.requiredFlags = req_flags; // ex) host visible
    alloc_ci.preferredFlags = pref_flags; // ex) host coherent, host cached

    // throw if failed
    // until residency management implemented
    // need to query memory status before creation
    check(vmaCreateBuffer(
        _factory.vma_allocator( ),
        &buffer_ci,
        &alloc_ci,
        &_handle,
        &_alloc,
        &_alloc_info
    ));

    // get BDA
    if ( buffer_usage_flags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) {
        VkBufferDeviceAddressInfo bda_info { };
        bda_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        bda_info.buffer = _handle;
        _device_address =
            vkGetBufferDeviceAddress(_factory.device( ), &bda_info);

        if ( _device_address == 0 ) {
            // bda fails
            // can have several reasons...
            // but mostly happens when the instance does not han bda feature...
            exit_with_message(
                "Error : BDA failed. check feature activation or support."
            );
        }
    }

    // for debugging, set name of the object
#ifdef ENABLE_VULKAN_VALIDATION
    set_buffer_debug_name(_factory.device( ), _handle, name.data( ));
#endif
}

VBuffer::~VBuffer( ) {
    // unmap before deletion
    // resource will live until synchronization
    if ( _alloc_info.pMappedData != nullptr ) {
        vmaUnmapMemory(_factory.vma_allocator( ), _alloc);
    }
    // push resources to the deletion queue
    // allocation and buffer
    if ( _alloc != nullptr ) {
        _factory.get_current_deletion_queue( ).enque(
            std::make_pair(_alloc, _handle)
        );
    }
}

// Movable
VBuffer::VBuffer(VBuffer&& other) :
    _factory(other._factory),
    _alloc(other._alloc),
    _handle(other._handle),
    _device_address(other._device_address),
    _alloc_info(other._alloc_info),
    _buffer_usage_flags(other._buffer_usage_flags),
    _mem_usage(other._mem_usage) {
    // nullify
    other._alloc = nullptr;
    other._handle = VK_NULL_HANDLE;
    other._device_address = 0;
    other._buffer_usage_flags = 0;
    other._mem_usage = { };
}

VBuffer& VBuffer::operator=(VBuffer&& other) {
    if ( this != &other ) {
        std::swap(other._alloc, _alloc);
        std::swap(other._handle, _handle);
        std::swap(other._device_address, _device_address);
        std::swap(other._alloc_info, _alloc_info);
        std::swap(other._buffer_usage_flags, _buffer_usage_flags);
        std::swap(other._mem_usage, _mem_usage);
    }
    return *this;
}

void VBuffer::flush(
    VkDeviceSize size = VK_WHOLE_SIZE,
    VkDeviceSize offset = 0
) {
    // if the buffer is not host coherent, fluse needed.
    // call after all memcpy called
    // but on PC, all host visible memory will be host coherent...
    vmaFlushAllocation(_factory.vma_allocator( ), _alloc, offset, size);
}

/*
    copy memory from system to gpu

    copy and flush(if needed) in one call
    memory mapping will be called only if it is needed
    so, if we map the memory already, mapping wont happen again
    if there is no mapped pointer, than this one can cause overhead

    Because of using allocator, vmaCopyMemory function holds mutex lock internally.
    can cause overhead.
*/
void VBuffer::write_back(
    void const* src,
    VkDeviceSize write_size,
    VkDeviceSize dst_offset = 0
) {
    vmaCopyMemoryToAllocation(
        _factory.vma_allocator( ),
        src,
        _alloc,
        dst_offset,
        write_size
    );
}

void VBuffer::invalidate(
    VkDeviceSize size = VK_WHOLE_SIZE,
    VkDeviceSize offset = 0
) {
    vmaInvalidateAllocation(_factory.vma_allocator( ), _alloc, offset, size);
}

/*
    copy memory from gpu to system

    invalidate(if needed) and read in one call
    memory mapping will be called only if it is needed
    so, if we map the memory already, mapping wont happen again
    if there is no mapped pointer, than this one can cause overhead

    Because of using allocator, vmaCopyMemory function holds mutex lock internally.
    can cause overhead.
*/
void VBuffer::read_back(
    void* dst,
    VkDeviceSize read_size,
    VkDeviceSize src_offset = 0
) {
    vmaCopyAllocationToMemory(
        _factory.vma_allocator( ),
        _alloc,
        src_offset,
        dst,
        read_size
    );
}
