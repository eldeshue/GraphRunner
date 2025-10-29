
#include "./DeviceImpl.h"

// NOLINTBEGIN
#include "Volk/volk.h"
// NOLINTEND

using namespace GraphRunner::Rhi::Impl;

// Physical device is the factory
DeviceImpl::DeviceImpl( ) : _dvc { } {}

DeviceImpl::~DeviceImpl( ) {
    vkDestroyDevice(_dvc, nullptr);
}
