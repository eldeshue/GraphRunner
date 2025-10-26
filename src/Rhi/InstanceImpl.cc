
#include "./InstanceImpl.h"

#include <GraphRunner/Rhi/PhysicalDevice.h>
#include <Logger.h>
#include <Util.h>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <set>
#include <string_view>

#include "./PhysicalDeviceImpl.h"
#include "RhiConfig.h"

// NOLINTBEGIN
#include "Volk/volk.h"

#define VP_USE_OBJECT
#include <VulkanProfiles/vulkan_profiles.hpp>
// NOLINTEND

using namespace GraphRunner::Rhi::Impl;
using namespace GraphRunner::Util;

VkResult InstanceImpl::volk_init_result = volkInitialize( );

static void check_profile_support(
    VpCapabilities const& cap,
    VpProfileProperties const& profile
) {
    VkBool32 is_supported = VK_FALSE;
    check(vpGetInstanceProfileSupport(cap, nullptr, &profile, &is_supported));
    if ( is_supported != VK_TRUE ) {
        throw_with_message(
            std::runtime_error(
                "Current profile is not supported at the instance level"
            ),
            "Current profile( {} ) is not supported at the instance level",
            RHI_VULKAN_PROFILE_NAME
        );
    }
}

static bool check_portability_support( ) {
    uint32_t cnt = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &cnt, nullptr);
    std::vector<VkExtensionProperties> supported_ext(cnt);
    VkResult const query_result = vkEnumerateInstanceExtensionProperties(
        nullptr,
        &cnt,
        supported_ext.data( )
    );
    return (
        std::find_if(
            supported_ext.begin( ),
            supported_ext.end( ),
            [](VkExtensionProperties const& ext_prop) {
                return strcmp(
                           ext_prop.extensionName,
                           VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
                       )
                    == 0; // name compare, same
            }
        )
        != supported_ext.end( )
    );
}

static void check_ext_support(std::vector<char const*> const& required_ext_names
) {
    // get number of supported ext
    uint32_t cnt = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &cnt, nullptr);
    if ( cnt < required_ext_names.size( ) ) {
        throw_with_message(
            std::runtime_error("Multiple required extensions not supported"),
            "Multiple required extensions not supported"
        );
    }
    // query supported ext
    std::vector<VkExtensionProperties> supported_ext(cnt);
    VkResult const query_result = vkEnumerateInstanceExtensionProperties(
        nullptr,
        &cnt,
        supported_ext.data( )
    );
    // check all required ext supported
    if ( query_result == VK_SUCCESS ) {
        for ( auto const& ext_prop : required_ext_names ) {
            std::string_view required_ext_name(ext_prop);
            if ( std::find_if(
                     supported_ext.begin( ),
                     supported_ext.end( ),
                     [required_ext_name](VkExtensionProperties const& ext_prop
                     ) {
                         return required_ext_name
                             == std::string_view(ext_prop.extensionName);
                     }
                 )
                 == supported_ext.end( ) ) {
                // required extension not found among supported extensions
                // exit
                throw_with_message(
                    std::runtime_error("Required extension not supported"),
                    "Required extension not supported : {}",
                    required_ext_name
                );
            }
        }
    }
}

static void
check_layer_support(std::vector<char const*> const& required_layer_names) {
    // get number of supported ext
    uint32_t cnt = 0;
    vkEnumerateInstanceLayerProperties(&cnt, nullptr);
    if ( cnt < required_layer_names.size( ) ) {
        throw_with_message(
            std::runtime_error("Multiple required layers not supported"),
            "Multiple required layers not supported"
        );
    }
    // query supported ext
    std::vector<VkLayerProperties> supported_layer(cnt);
    VkResult const query_result =
        vkEnumerateInstanceLayerProperties(&cnt, supported_layer.data( ));
    // check all required ext supported
    if ( query_result == VK_SUCCESS ) {
        for ( auto const& layer_prop : required_layer_names ) {
            std::string_view required_layer_name(layer_prop);
            if ( std::find_if(
                     supported_layer.begin( ),
                     supported_layer.end( ),
                     [required_layer_name](VkLayerProperties const& layer_prop
                     ) {
                         return required_layer_name
                             == std::string_view(layer_prop.layerName);
                     }
                 )
                 == supported_layer.end( ) ) {
                // required extension not found among supported extensions
                // exit
                throw_with_message(
                    std::runtime_error("Reauired layer not supported"),
                    "Required layer not supported : {}",
                    required_layer_name
                );
            }
        }
    }
}

static std::vector<char const*> get_final_extension(
    std::vector<std::string_view> const& ext_names,
    VpCapabilities const& cap,
    VpProfileProperties const& profile
) {
    std::set<std::string_view> merge_set(ext_names.begin( ), ext_names.end( ));
    uint32_t cnt = 0;
    // get number of ext in the profile
    vpGetProfileInstanceExtensionProperties(
        cap,
        &profile,
        nullptr,
        &cnt,
        nullptr
    );
    std::vector<VkExtensionProperties> profile_ext(cnt);
    // get ext from profile
    check(vpGetProfileInstanceExtensionProperties(
        cap,
        &profile,
        nullptr,
        &cnt,
        profile_ext.data( )
    ));
    for ( auto const& ext_prop : profile_ext ) {
        // must be null-terminated
        merge_set.insert(ext_prop.extensionName);
    }

#ifdef ENABLE_VULKAN_VALIDATION
    // add debug utils ext, VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    merge_set.insert(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    // transform
    std::vector<char const*> result(merge_set.size( ));
    std::transform(
        merge_set.begin( ),
        merge_set.end( ),
        result.begin( ),
        [](std::string_view sv) { return sv.data( ); }
    );
    check_ext_support(result);
    return result;
}

static std::vector<char const*>
get_final_layer(std::vector<std::string_view> const& l_names) {
    // there is no layer in the profile, do nothing
    // transform
    std::vector<char const*> result(l_names.size( ));
    std::transform(
        l_names.begin( ),
        l_names.end( ),
        result.begin( ),
        [](std::string_view sv) { return sv.data( ); }
    );
#ifdef ENABLE_VULKAN_VALIDATION
    // add validation layer, "VK_LAYER_KHRONOS_validation"
    if ( std::find(
             l_names.begin( ),
             l_names.end( ),
             "VK_LAYER_KHRONOS_validation"
         )
         == l_names.end( ) ) {
        result.push_back("VK_LAYER_KHRONOS_validation");
    }
#endif
    check_layer_support(result);
    return result;
}

#ifdef ENABLE_VULKAN_VALIDATION
// debug call-back function
// will be called when vulkan debug option enabled
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_logging_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    VkDebugUtilsMessengerCallbackDataEXT const* pcallback_data,
    void* pUserData
) {
    // format message
    std::stringstream debug_message;
    if ( pcallback_data->pMessageIdName ) {
        debug_message << "[" << pcallback_data->messageIdNumber << "]["
                      << pcallback_data->pMessageIdName
                      << "] : " << pcallback_data->pMessage;
    } else {
        debug_message << "[" << pcallback_data->messageIdNumber
                      << "] : " << pcallback_data->pMessage;
    }

    // print to the log files
    if ( message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT ) {
        print_log("[VERBOSE] {}", debug_message.str( ));
    } else if ( message_severity
                & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ) {
        print_log("[INFO] {}", debug_message.str( ));
    } else if ( message_severity
                & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ) {
        print_log("[WARNING] {}", debug_message.str( ));
    } else if ( message_severity
                & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ) {
        exit_with_message("[ERROR] {}", debug_message.str( ));
    }
    return VK_FALSE;
}

static void set_debug_messenger_ci(VkDebugUtilsMessengerCreateInfoEXT& ci) {
    // vulkan related debug options
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    //  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    //  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
    ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    // VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ci.pfnUserCallback = debug_messenger_logging_callback;
}
#endif

static void set_vp_func_instance_with_volk(VpVulkanFunctions& functions) {
    // set func ptr with volk loaded functions
    // instance has not been created,
    // so only initialize instance related functions
    functions.EnumerateInstanceVersion = vkEnumerateInstanceVersion;
    functions.EnumerateInstanceExtensionProperties =
        vkEnumerateInstanceExtensionProperties;
    functions.GetInstanceProcAddr = vkGetInstanceProcAddr;
    functions.CreateInstance = vkCreateInstance;
    /*
    // device related vp functions, not loaded yet
    functions.CreateDevice = vkCreateDevice;
    functions.GetDeviceProcAddr = vkGetDeviceProcAddr;
    functions.EnumerateDeviceExtensionProperties =
        vkEnumerateDeviceExtensionProperties;
    functions.GetPhysicalDeviceFeatures2 = vkGetPhysicalDeviceFeatures2;
    functions.GetPhysicalDeviceProperties2 = vkGetPhysicalDeviceProperties2;
    functions.GetPhysicalDeviceFormatProperties2 =
        vkGetPhysicalDeviceFormatProperties2;
    functions.GetPhysicalDeviceQueueFamilyProperties2 =
        vkGetPhysicalDeviceQueueFamilyProperties2;
     */
}

InstanceImpl::InstanceImpl(
    std::string_view app_name,
    std::string_view engine_name,
    std::vector<std::string_view> const& required_ext_names,
    std::vector<std::string_view> const& required_laye_names
) :
    _instance { }
#ifdef ENABLE_VULKAN_VALIDATION
    ,
    _dbg_messenger { }
#endif
{
    // volk init, init volk loader
    if ( volk_init_result != VK_SUCCESS ) {
        throw_with_message(
            std::runtime_error("volk initializaion failed"),
            "volk initialization failed"
        );
    }

    // ----------------- instance creation -------------- //
    // profile creation
    VpVulkanFunctions volk_initialized_functions = { };
    set_vp_func_instance_with_volk(volk_initialized_functions);

    VpCapabilitiesCreateInfo vp_cap_ci = { };
    vp_cap_ci.apiVersion = RHI_VULKAN_API_VERSION;
    vp_cap_ci.flags = 0;
    vp_cap_ci.pVulkanFunctions = &volk_initialized_functions;

    VpCapabilities vp_cap = { };
    // does not check vpCreateCap.. because volk does not fully loaded...
    vpCreateCapabilities(&vp_cap_ci, nullptr, &vp_cap);
    VpProfileProperties profile {
        RHI_VULKAN_PROFILE_NAME,
        RHI_VULKAN_PROFILE_SPEC_VERSION
    };
    check_profile_support(vp_cap, profile);

    // merge extension from profile
    auto layers = get_final_layer(required_laye_names);
    auto extensions = get_final_extension(required_ext_names, vp_cap, profile);

#ifdef ENABLE_VULKAN_VALIDATION
    // debug messenger creation info
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_ci { };
    set_debug_messenger_ci(debug_messenger_ci);
#endif
#ifdef ENABLE_VULKAN_PORTABILITY
    bool const is_portability_supported = check_portability_support( );
    if ( is_portability_supported ) {
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    }
#endif

    // app creation info
    VkApplicationInfo app_info { };
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name.data( );
    app_info.pEngineName = engine_name.data( );
    app_info.apiVersion = RHI_VULKAN_PROFILE_MIN_API_VERSION;

    // instance creation info
    VkInstanceCreateInfo instance_ci { };
    instance_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_ci.pApplicationInfo = &app_info;
#ifdef ENABLE_VULKAN_VALIDATION
    // chaining creation info for debugging create instance
    debug_messenger_ci.pNext = instance_ci.pNext;
    instance_ci.pNext = &debug_messenger_ci;
#endif
#ifdef ENABLE_VULKAN_PORTABILITY
    if ( is_portability_supported ) {
        instance_ci.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif
    instance_ci.enabledLayerCount = layers.size( );
    instance_ci.ppEnabledLayerNames = layers.data( );
    instance_ci.enabledExtensionCount = extensions.size( );
    instance_ci.ppEnabledExtensionNames = extensions.data( );
    check(vkCreateInstance(&instance_ci, nullptr, &_instance));

    // load api functions with volk
    volkLoadInstance(_instance);

    // ----------------- instance creation -------------- //

#ifdef ENABLE_VULKAN_VALIDATION
    // messenger create
    check(vkCreateDebugUtilsMessengerEXT(
        _instance,
        &debug_messenger_ci,
        nullptr,
        &_dbg_messenger
    ));
#endif
}

InstanceImpl::~InstanceImpl( ) {
#ifdef ENABLE_VULKAN_VALIDATION
    vkDestroyDebugUtilsMessengerEXT(_instance, _dbg_messenger, nullptr);
#endif
    vkDestroyInstance(_instance, nullptr);
    // volk finalize is not necessary
}

static std::uint64_t scoring_gpu_type(VkPhysicalDeviceType type) {
    switch ( type ) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return 5;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return 4;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: // not for local
            return 3;
        case VK_PHYSICAL_DEVICE_TYPE_CPU: // SW implemented
            return 2;
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return 1;
        default: // unreachable
            return 0;
    }
}

// scoring gpu for general purpose, pc desktop enviornment
static std::uint64_t scoring_gpu(VkPhysicalDevice const& gpu) {
    std::uint64_t result = 0;

    // gpu type
    // prefer discrete gpu
    VkPhysicalDeviceProperties2 prop2 { };
    vkGetPhysicalDeviceProperties2(gpu, &prop2);
    result |= (scoring_gpu_type(prop2.properties.deviceType) << 60);

    // vram size
    // prefer large size
    VkPhysicalDeviceMemoryProperties2 mem_prop2 { };
    vkGetPhysicalDeviceMemoryProperties2(gpu, &mem_prop2);
    VkDeviceSize vram_size = 0;
    for ( uint32_t i = 0; i < mem_prop2.memoryProperties.memoryHeapCount;
          ++i ) {
        VkMemoryHeap const& heap = mem_prop2.memoryProperties.memoryHeaps[i];
        if ( heap.flags
             & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ) // check local bit flag
        {
            vram_size += heap.size;
        }
    }
    result |= vram_size;

    return result;
}

std::optional<GraphRunner::Rhi::PhysicalDevice>
InstanceImpl::create_single_physical_device_with_best_vram( ) const {
    // factory function, create empty object
    GraphRunner::Rhi::PhysicalDevice result;
    result._impl = new PhysicalDeviceImpl( );

    // fill the object
    // pick discrete gpu, with largest vram
    uint32_t gpuCount = 0;
    check(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr));
    if ( gpuCount == 0 ) {
        return std::nullopt;
    }
    std::vector<VkPhysicalDevice> devices(gpuCount);
    check(vkEnumeratePhysicalDevices(_instance, &gpuCount, devices.data( )));

    // sort gpu by property
    // discrete, integrated, cpu, virtual, other
    std::vector<std::pair<std::uint64_t, VkPhysicalDevice>> sort_buffer(gpuCount
    );
    std::transform(
        devices.begin( ),
        devices.end( ),
        sort_buffer.begin( ),
        [](VkPhysicalDevice const& gpu) {
            return std::make_pair(scoring_gpu(gpu), gpu);
        }
    );
    // sort descending order
    std::sort(
        sort_buffer.begin( ),
        sort_buffer.end( ),
        std::greater<std::pair<std::uint64_t, VkPhysicalDevice>>( )
    );

    // init
    // highest score at index 0
    result._impl->_pdvc = sort_buffer[0].second;

    // return
    return result;
}
