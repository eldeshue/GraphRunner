
#pragma once

#include <cstdint>

namespace GraphRunner {
namespace RenderModule {
    // common interface
    class RenderSystem {
      private:
        RenderSystem(RenderSystem const&) = delete;
        RenderSystem& operator=(RenderSystem const&) = delete;

      protected:
        RenderSystem( ) = default;
        RenderSystem(RenderSystem&&) = default;
        RenderSystem& operator=(RenderSystem&&) = default;

      public:
        virtual ~RenderSystem( ) = 0;

        // must be called at frame start on render thread
        virtual void reset(uint32_t cur_frame_index) = 0;

        // must be called during command dispatch
        virtual void execute(uint32_t cur_frame_index, void* param) = 0;

        // build RDG node
        // return lambda...
    };

    inline RenderSystem::~RenderSystem( ) {}
} // namespace RenderModule
} // namespace GraphRunner
