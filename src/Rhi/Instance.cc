
#include <GraphRunner/Rhi/Instance.h>

#include "InstanceImpl.h"

using namespace GraphRunner::Rhi;

Instance::Instance(
    std::string_view app_name,
    std::string_view engine_name,
    std::vector<std::string_view> const& required_ext_names,
    std::vector<std::string_view> const& required_laye_names
) :
    impl(new Impl::InstanceImpl(
        app_name,
        engine_name,
        required_ext_names,
        required_laye_names
    )) {}

Instance::~Instance( ) {
    delete impl;
    impl = nullptr;
}

Instance::Instance(Instance&& other) noexcept : impl(other.impl) {
    other.impl = nullptr;
}

Instance& Instance::operator=(Instance&& other) noexcept {
    if ( this != &other ) {
        std::swap(this->impl, other.impl);
    }
    return *this;
}
