
#include "./DeviceImpl.h"

#include <GraphRunner/Rhi/Queue.h>

#include <tuple>

#include "./QueueImpl.h"

// NOLINTBEGIN
#include "Volk/volk.h"
// NOLINTEND

using namespace GraphRunner::Rhi::Impl;
using namespace GraphRunner::Rhi;

// Physical device is the factory
DeviceImpl::DeviceImpl( ) : _handle { } {}

DeviceImpl::~DeviceImpl( ) {
    vkDestroyDevice(_handle, nullptr);
}

namespace {
using QueuInfo =
    std::tuple<VkQueueFamilyProperties, VkDeviceQueueCreateInfo, uint32_t>;

int32_t find_queue_family_info_index(
    std::vector<QueuInfo> const& queue_infos,
    VkQueueFlags type
) {
    // find queue family and select
    for ( uint32_t i = 0; i < queue_infos.size( ); ++i ) {
        auto const& [queue_family_prop, queue_ci, used_cnt] = queue_infos[i];
        if ( (queue_family_prop.queueFlags & type) == type // type match
             && (used_cnt < queue_family_prop.queueCount
             ) ) { // create available
            return i;
        }
    }

    // if failed, choose graphics queue instead
    for ( uint32_t i = 0; i < queue_infos.size( ); ++i ) {
        auto const& [queue_family_prop, queue_ci, used_cnt] = queue_infos[i];
        if ( (queue_family_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                 == VK_QUEUE_GRAPHICS_BIT // type match
             && (used_cnt < queue_family_prop.queueCount
             ) ) { // create available
            return i;
        }
    }

    // not found, -1
    return -1;
}
} // namespace

std::optional<Queue> DeviceImpl::create_queue_with_flags(VkQueueFlags flags) {
    Queue result;
    result._impl = new QueueImpl( );

    // find graphic queue family
    int32_t i = find_queue_family_info_index(_queue_infos, flags);
    if ( i == -1 ) {
        return std::nullopt;
    }
    auto& [queue_family_prop, queue_ci, used_cnt] = _queue_infos[i];
    result._impl->_info = &queue_family_prop;

    // used queue check
    if ( used_cnt == queue_ci.queueCount ) { // all queue used
        return std::nullopt;
    }

    // init queue
    VkDeviceQueueInfo2 ci = { };
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
    ci.pNext = nullptr;
    ci.flags = 0;
    ci.queueFamilyIndex = queue_ci.queueFamilyIndex;
    ci.queueIndex = used_cnt;
    vkGetDeviceQueue2(_handle, &ci, &result._impl->_handle);

    // increase counter
    used_cnt++;
    return result;
}

std::optional<Queue> DeviceImpl::create_graphics_queue( ) {
    return create_queue_with_flags(VK_QUEUE_GRAPHICS_BIT);
}

std::optional<Queue> DeviceImpl::create_compute_queue( ) {
    return create_queue_with_flags(VK_QUEUE_COMPUTE_BIT);
}

std::optional<Queue> DeviceImpl::create_transfer_queue( ) {
    return create_queue_with_flags(VK_QUEUE_TRANSFER_BIT);
}
