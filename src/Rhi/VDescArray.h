
#pragma once

#include <atomic>
#include <optional>
#include <vector>

#include "./GraphicsApiCore.h"

namespace GraphRunner {
namespace Rhi {
    class VDescArray {
      private:
        VDescArray( ) = delete;
        VDescArray(VDescArray const&) = delete;
        VDescArray& operator=(VDescArray const&) = delete;

        VkDescriptorType _type; // type of descriptor
        size_t _cap; // max limit of activated index, not resizable, immutable

        SpinLock _m;
        size_t _free_stack_ptr;
        std::vector<uint32_t> _free_stack;

      public:
        VDescArray(VkDescriptorType type, size_t cap);
        VDescArray(VDescArray&& other) noexcept;
        VDescArray& operator=(VDescArray&& other) noexcept;

        // getter
        VkDescriptorType type( ) const {
            return _type;
        }

        size_t cap( ) const {
            return _cap;
        }

        size_t size( ) const {
            return _cap - _free_stack_ptr;
        }

        std::optional<uint32_t> allocate_index( );
        void free_index(uint32_t index);
    };
} // namespace Rhi
} // namespace GraphRunner
