
#ifndef UTILS_H
#define UTILS_H

// NOLINTBEGIN
#include "./GraphicsApiCore.h"
// NOLINTEND

#include <exception>
#include <string>

#include "Logger.h"

namespace GraphRunner {
namespace Util {

    std::string get_result_string(VkResult errorCode);

    void check(VkResult result);

    void set_vp_capabilities(VpCapabilities& cap);

    constexpr VkDeviceSize
    calculate_aligned_value(VkDeviceSize target_value, VkDeviceSize alignment);

    // simple implementation of spin lock, minimize context switching
    // implementation is from atomic_flag in cppreference
    class SpinLock {
      private:
        std::atomic_flag _mutex_flag { };

      public:
        void lock( ) noexcept {
            // if flag is clear, set and return false(prev value)
            // if flag is already set, return true(prev value)
            while ( _mutex_flag.test_and_set(std::memory_order_acquire) ) {
                // hybrid spin lock, effective busy waiting
                // wake up by notify one in unlock
                _mutex_flag.wait(true, std::memory_order_relaxed);
            }
        }

        bool try_lock( ) noexcept {
            return !_mutex_flag.test_and_set(std::memory_order_acquire);
        }

        void unlock( ) noexcept {
            _mutex_flag.clear(std::memory_order_release);
            _mutex_flag.notify_one( ); // signal one waiting thread
        }
    };
} // namespace Util
} // namespace GraphRunner

#endif
