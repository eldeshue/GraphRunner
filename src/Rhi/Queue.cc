
#include <GraphRunner/Rhi/Queue.h>

#include <utility>

#include "./QueueImpl.h"

using namespace GraphRunner::Rhi;

Queue::Queue( ) : _impl { } {}

Queue::~Queue( ) {
    delete _impl;
    _impl = nullptr;
}

Queue::Queue(Queue&& other) noexcept : _impl(other._impl) {
    other._impl = nullptr;
}

Queue& Queue::operator=(Queue&& other) noexcept {
    if ( this != &other ) {
        std::swap(_impl, other._impl);
    }
    return *this;
}
