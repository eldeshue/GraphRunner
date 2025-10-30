
#include "./DeviceImpl.h"

// NOLINTBEGIN
#include "Volk/volk.h"
// NOLINTEND

using namespace GraphRunner::Rhi::Impl;

// Physical device is the factory
DeviceImpl::DeviceImpl( ) :
    _handle { },
    graphic_queue_limit { },
    graphic_queue_cnt { },
    compute_queue_limit { },
    compute_queue_cnt { },
    transfer_queue_limit { },
    transfer_queue_cnt { } {}

DeviceImpl::~DeviceImpl( ) {
    vkDestroyDevice(_handle, nullptr);
}

uint32_t DeviceImpl::get_graphic_queue_limit( ) const {
    return graphic_queue_limit;
}

uint32_t DeviceImpl::get_graphic_queue_cnt( ) const {
    return graphic_queue_cnt;
}

uint32_t DeviceImpl::get_compute_queue_limit( ) const {
    return compute_queue_limit;
}

uint32_t DeviceImpl::get_compute_queue_cnt( ) const {
    return compute_queue_cnt;
}

uint32_t DeviceImpl::get_transfer_queue_limit( ) const {
    return transfer_queue_limit;
}

uint32_t DeviceImpl::get_transfer_queue_cnt( ) const {
    return transfer_queue_cnt;
}
