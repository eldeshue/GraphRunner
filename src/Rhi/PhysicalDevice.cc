
#include <GraphRunner/Rhi/Device.h>
#include <GraphRunner/Rhi/PhysicalDevice.h>

#include <optional>
#include <string_view>
#include <utility>
#include <vector>

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

void PhysicalDevice::log_info( ) const {
    return _impl->log_info( );
}

std::optional<Device>
PhysicalDevice::create_logical_device_with_single_graphic_queue(
    std::vector<std::string_view> const& ext_names
) const {
    return _impl->create_logical_device_with_single_graphic_queue(ext_names);
}
