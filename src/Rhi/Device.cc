
#include <GraphRunner/Rhi/Device.h>
#include <GraphRunner/Rhi/Queue.h>

#include <cstddef>
#include <optional>
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

std::optional<Queue> Device::create_graphics_queue( ) {
    return _impl->create_graphics_queue( );
}

std::optional<Queue> Device::create_compute_queue( ) {
    return _impl->create_compute_queue( );
}

std::optional<Queue> Device::create_transfer_queue( ) {
    return _impl->create_transfer_queue( );
}
