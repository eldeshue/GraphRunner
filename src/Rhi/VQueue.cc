
#include "./VQueue.h"

using namespace GraphRunner::Rhi;

VQueue::VQueue( ) : _handle { }, _info { } {}

VQueue::VQueue(VQueue&& other) noexcept :
    _handle(other._handle), _info(other._info) {
    other._handle = VK_NULL_HANDLE;
    _info = nullptr;
}

VQueue& VQueue::operator=(VQueue&& other) noexcept {
    if ( this != &other ) {
        std::swap(_handle, other._handle);
        std::swap(_info, other._info);
    }
    return *this;
}

VQueue::~VQueue( ) {
    // queue belongs to the Device, no need to erase
}

VkQueue VQueue::handle( ) const {
    return _handle;
}
