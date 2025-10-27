
#include <GraphRunner/Rhi/Instance.h>
#include <GraphRunner/Rhi/PhysicalDevice.h>

#include "./PhysicalDeviceImpl.h"

using namespace GraphRunner::Rhi;

PhysicalDevice::PhysicalDevice( ) : _impl(nullptr) {
    // factory will fill the content
}

PhysicalDevice::~PhysicalDevice( ) {
    delete _impl;
    _impl = nullptr;
}

PhysicalDevice::PhysicalDevice(PhysicalDevice&& other) noexcept :
    _impl(other._impl) {
    other._impl = nullptr;
}

PhysicalDevice& PhysicalDevice::operator=(PhysicalDevice&& other) noexcept {
    if ( this != &other ) {
        std::swap(_impl, other._impl);
    }
    return *this;
}
