
#ifndef RHI_INSTANCE
#define RHI_INSTANCE

#include <string_view>
#include <vector>

namespace GraphRunner {
namespace Rhi {
    namespace Impl {
        class InstanceImpl;
    }

    class Instance {
      private:
        // no default, no copy
        Instance( ) = delete;
        Instance(Instance const&) = delete;
        Instance& operator=(Instance const&) = delete;

        Impl::InstanceImpl* _impl; // pointer to implementation

      public:
        Instance(
            std::string_view app_name,
            std::string_view engine_name,
            std::vector<std::string_view> const& required_ext_names,
            std::vector<std::string_view> const& required_laye_names
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
