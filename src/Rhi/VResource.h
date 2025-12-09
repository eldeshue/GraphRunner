
#pragma once

namespace GraphRunner {
namespace Rhi {
    // common interface
    class VResource {
      private:
        VResource(VResource const&) = delete;
        VResource& operator=(VResource const&) = delete;

      protected:
        VResource( ) = default;
        VResource(VResource&&) = default;
        VResource& operator=(VResource&&) = default;

      public:
        virtual ~VResource( ) = 0;
    };

    inline VResource::~VResource( ) {}
} // namespace Rhi
} // namespace GraphRunner
