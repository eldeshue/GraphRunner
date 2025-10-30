
#include <GraphRunner/Rhi/Device.h>

#include <cstddef>
#include <utility>

#include "./DeviceImpl.h"

using namespace GraphRunner::Rhi;

Device::Device( ) : _impl(nullptr) {}

Device::~Device( ) {
    delete _impl;
    _impl = nullptr;
}

Device::Device(Device&& other) noexcept : _impl(other._impl) {
    other._impl = nullptr;
}

Device& Device::operator=(Device&& other) noexcept {
    if ( this != &other ) {
        std::swap(this->_impl, other._impl);
    }
    return *this;
}

uint32_t Device::get_graphic_queue_limit( ) const {
    return _impl->get_graphic_queue_limit( );
}

uint32_t Device::get_graphic_queue_cnt( ) const {
    return _impl->get_graphic_queue_cnt( );
}

uint32_t Device::get_compute_queue_limit( ) const {
    return _impl->get_compute_queue_limit( );
}

uint32_t Device::get_compute_queue_cnt( ) const {
    return _impl->get_compute_queue_cnt( );
}

uint32_t Device::get_transfer_queue_limit( ) const {
    return _impl->get_transfer_queue_limit( );
}

uint32_t Device::get_transfer_queue_cnt( ) const {
    return _impl->get_transfer_queue_cnt( );
}
