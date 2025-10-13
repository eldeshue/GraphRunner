
#ifndef RHI_INSTANCE
#define RHI_INSTANCE

// vulkan
#include <volk.h>

#include <string_view>
#include <vector>

namespace GraphRunner {
namespace Rhi {
    class Instance {
      private:
        // disable copy constructor
        Instance(Instance const&) = delete;
        Instance& operator=(Instance const&) = delete;

        // volk load result
        static VkResult volk_init_result;

        // instance or DXGIFactory
        using InstanceHandle = VkInstance;
        InstanceHandle _instance;

        using DebugHandle = VkDebugUtilsMessengerEXT;
        DebugHandle _dbg_messenger;

      public:
        // default, profile 2024 roadmap
        Instance(
            std::string_view app_name,
            std::string_view engine_name,
            std::vector<char const*>& extensions,
            std::vector<char const*>& layers
        );
        ~Instance( );

        // movable
        Instance(Instance&& other) noexcept;
        Instance& operator=(Instance&& other) noexcept;

        //
    };
} // namespace Rhi
} // namespace GraphRunner

#endif
