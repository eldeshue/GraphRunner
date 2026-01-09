
#pragma once

namespace GraphRunner {
namespace Rhi {
    // common interface
    class RenderResource {
      private:
        RenderResource(RenderResource const&) = delete;
        RenderResource& operator=(RenderResource const&) = delete;

      protected:
        RenderResource( ) = default;
        RenderResource(RenderResource&&) = default;
        RenderResource& operator=(RenderResource&&) = default;

      public:
        virtual ~RenderResource( ) = 0;
    };

    inline RenderResource::~RenderResource( ) {}
} // namespace Rhi
} // namespace GraphRunner
