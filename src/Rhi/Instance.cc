
#include <GraphRunner/Rhi/Instance.h>
#include <GraphRunner/Rhi/PhysicalDevice.h>

#include <optional>

#include "InstanceImpl.h"

using namespace GraphRunner::Rhi;

Instance::Instance(
    std::string_view app_name,
    std::string_view engine_name,
    std::vector<std::string_view> const& required_ext_names,
    std::vector<std::string_view> const& required_laye_names
) :
    _impl(new Impl::InstanceImpl(
        app_name,
        engine_name,
        required_ext_names,
        required_laye_names
    )) {}

Instance::~Instance( ) {
    delete _impl;
    _impl = nullptr;
}

Instance::Instance(Instance&& other) noexcept : _impl(other._impl) {
    other._impl = nullptr;
}

Instance& Instance::operator=(Instance&& other) noexcept {
    if ( this != &other ) {
        std::swap(this->_impl, other._impl);
    }
    return *this;
}

std::optional<PhysicalDevice>
Instance::create_single_physical_device_with_best_vram( ) const {
    return _impl->create_single_physical_device_with_best_vram( );
}
