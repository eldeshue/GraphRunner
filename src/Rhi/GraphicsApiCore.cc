
// volk related options
// implementation of volk
// #define VOLK_IMPLEMENTATION

// vma related options
#define VMA_VULKAN_VERSION 1003000 // vulkan 1.3
// becaulse of using volk, vulkan functions will be manually loaded
// use vmaImportVulkanFunctionsFromVolk()
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
// implementation of vma
#define VMA_IMPLEMENTATION

// common api header
#include "./GraphicsApiCore.h"
