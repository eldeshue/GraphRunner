
#include <GraphRunner/Rhi/Device.h>

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
