
#ifndef GRAPHICS_API_CORE_H
#define GRAPHICS_API_CORE_H

// platform configuration
// for windows
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
// for linux
#elif defined(__linux__)
// for Mac Os
#elif defined(__APPLE__)
#endif

/* --------------- vulkan related configuration --------------- */

// platform configuration
#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
    // #define VK_USE_PLATFORM_WAYLAND_KHR  // for wayland server
    #define VK_USE_PLATFORM_XLIB_KHR // for x11 server
#elif defined(__APPLE__)
    #define VK_USE_PLATFORM_METAL_EXT
#endif

// vulkan api loader
#include <Volk/volk.h>

// vulkan profile
#define VP_USE_OBJECT
#include <VulkanProfiles/vulkan_profiles.hpp>

// VMA
// vulkan memory allocator
// #define VMA_STATIC_VULKAN_FUNCTIONS 0
// #define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
// #include "vk_mem_alloc.h"

// glfw3
// presenting rendering result
// #include <GLFW/glfw3.h>

#endif
