
#include "./VStagingHeap.h"

#include "Util.h"

using namespace GraphRunner::Rhi;
using namespace GraphRunner::Util;

VStagingHeap::VStagingHeap(
    VResourceManager& source,
    VkDeviceSize non_coherent_alignment,
    VkDeviceSize size
) :
    _cur_offset(0),
    _non_coherent_alignment(non_coherent_alignment),
    _buffer(
        source, // resource manager, vma allocator
        size, // size of buffer
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // for staging buffer
        VMA_MEMORY_USAGE_AUTO, // let vma choose memory type
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
            | VMA_ALLOCATION_CREATE_MAPPED_BIT, // for writeable, mapping
        0, // no require
        0, // no prefer
        true,
        "staging belt"
    ) {}

VStagingHeap::~VStagingHeap( ) {
    // no other resource to handle
}

VStagingHeap::VStagingHeap(VStagingHeap&& other) noexcept :
    _cur_offset(other._cur_offset.load( )),
    _non_coherent_alignment(other._non_coherent_alignment),
    _buffer(std::move(other._buffer)) {}

VStagingHeap& VStagingHeap::operator=(VStagingHeap&& other) noexcept {
    if ( this != &other ) {
        _cur_offset.store(other._cur_offset.load( ));
        std::swap(_non_coherent_alignment, other._non_coherent_alignment);
        _buffer = std::move(other._buffer); // move, cannot copy
    }
    return *this;
}

// check size with offset, move offset, copy data to buffer
// default alignment is nonCoherentAtomSize for cache coherent
// if copy from buffer to image, alignment must be optimalBufferCopyOffsetAlignment
std::optional<StagedData>
VStagingHeap::try_push(void* src, VkDeviceSize size, VkDeviceSize alignment) {
    // calculate aligned size
    // alignment must be bigger than non coherent atom size
    alignment = std::max(alignment, _non_coherent_alignment);

    // for CAS
    VkDeviceSize current_offset = _cur_offset.load( );
    VkDeviceSize aligned_start_offset = 0;
    VkDeviceSize new_offset = 0;

    // CAS loop
    while ( true ) {
        aligned_start_offset =
            calculate_aligned_value(current_offset, alignment);

        new_offset = aligned_start_offset + size;

        if ( new_offset > _buffer.size( ) ) {
            // cannot stage data, not enough capacity
            return std::nullopt;
        }

        // try CAS
        if ( _cur_offset.compare_exchange_weak(current_offset, new_offset) ) {
            break;
        }
        // cas failed, re enter the loop
    }

    // memory copy
    std::memcpy(
        static_cast<uint8_t*>(_buffer.mapped_ptr( )) + aligned_start_offset,
        src,
        size
    );

    // return info
    return StagedData {_buffer.handle( ), aligned_start_offset, size};
}
