
#include "./VDescArray.h"

#include <numeric>

#include "Util.h"

using namespace GraphRunner::Rhi;
using namespace GraphRunner::Util;

VDescArray::VDescArray(VkDescriptorType type, size_t cap) :
    _type(type), _cap(cap), _free_stack_ptr(cap), _free_stack(cap, 0) {
    // fill with 0..cap
    // allocate index from zero
    std::iota(_free_stack.rbegin( ), _free_stack.rend( ), 0);
}

VDescArray::VDescArray(VDescArray&& other) noexcept :
    _type(other._type),
    _cap(other._cap),
    _free_stack_ptr(other._free_stack_ptr),
    _free_stack(std::move(other._free_stack)) {}

VDescArray& VDescArray::operator=(VDescArray&& other) noexcept {
    if ( this != &other ) {
        _type = other._type;
        _cap = other._cap;
        _free_stack_ptr = other._free_stack_ptr;
        _free_stack = std::move(other._free_stack);
    }
    return *this;
}

std::optional<uint32_t> VDescArray::allocate_index( ) {
    // 단순한 stack pointer 하나만 운영하면 되는 로직
    // 따라서 별도의 mutex 대신, 간단한 splin lock을 구현하여 이를 활용함.
    // 단순 mutex는 context switching의 발동을 전제로 하기 때문에 무거움
    // 그렇다고, atomic 변수들로 lockless하게 구성하기는 구현 난이도가 too much하다 판단
    _m.lock( );
    if ( _free_stack_ptr == 0 ) {
        _m.unlock( );
        return std::nullopt;
    }
    uint32_t result = _free_stack[--_free_stack_ptr];
    _m.unlock( );
    return result;
}

void VDescArray::free_index(uint32_t index) {
    _m.lock( );
    if ( _free_stack_ptr < _cap ) {
        _free_stack[_free_stack_ptr++] = index;
    }
    _m.unlock( );
}
