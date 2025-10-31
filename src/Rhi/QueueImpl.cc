
#include "./QueueImpl.h"

using namespace GraphRunner::Rhi::Impl;

QueueImpl::QueueImpl( ) : _handle { }, _info { } {}

QueueImpl::~QueueImpl( ) {
    // queue belongs to the Device, no need to erase
}
