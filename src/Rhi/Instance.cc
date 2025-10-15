
#include "Instance.h"

#include <Logger.h>
#include <Util.h>

#include <algorithm>
#include <cstddef>
#include <set>
#include <string_view>

#include "RhiConfig.h"

#define VP_USE_OBJECT
#include <vulkan_profiles.hpp>

using namespace GraphRunner::Rhi;
using namespace GraphRunner::Util;

VkResult Instance::volk_init_result = volkInitialize( );

// TODO
// 2. linux 및 mac os를 위한 portability 활성화
// VK_KHR_portability_enumeration 연구 필요
// #define ENABLE_VULKAN_VALIDATION
Instance::Instance(
    std::string_view app_name,
    std::string_view engine_name,
    std::vector<std::string_view> const& required_ext_names,
    std::vector<std::string_view> const& required_laye_names
) :
    _instance { }, _dbg_messenger { } {
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
    set_vp_vulkan_func_with_volk(volk_initialized_functions);

    VpCapabilitiesCreateInfo vp_cap_ci = { };
    vp_cap_ci.apiVersion = RHI_VULKAN_API_VERSION;
    vp_cap_ci.flags = VP_PROFILE_CREATE_STATIC_BIT;
    vp_cap_ci.pVulkanFunctions = &volk_initialized_functions;

    VpCapabilities vp_cap = { };
    check(vpCreateCapabilities(&vp_cap_ci, nullptr, &vp_cap));
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

static void set_vp_vulkan_func_with_volk(VpVulkanFunctions& functions) {
    // set func ptr with volk loaded functions
    functions.GetInstanceProcAddr = vkGetInstanceProcAddr;
    functions.GetDeviceProcAddr = vkGetDeviceProcAddr;
    functions.EnumerateInstanceVersion = vkEnumerateInstanceVersion;
    functions.EnumerateInstanceExtensionProperties =
        vkEnumerateInstanceExtensionProperties;
    functions.EnumerateDeviceExtensionProperties =
        vkEnumerateDeviceExtensionProperties;
    functions.GetPhysicalDeviceFeatures2 = vkGetPhysicalDeviceFeatures2;
    functions.GetPhysicalDeviceProperties2 = vkGetPhysicalDeviceProperties2;
    functions.GetPhysicalDeviceFormatProperties2 =
        vkGetPhysicalDeviceFormatProperties2;
    functions.GetPhysicalDeviceQueueFamilyProperties2 =
        vkGetPhysicalDeviceQueueFamilyProperties2;
    functions.CreateInstance = vkCreateInstance;
    functions.CreateDevice = vkCreateDevice;
}

Instance::~Instance( ) {
#ifdef ENABLE_VULKAN_VALIDATION
    vkDestroyDebugUtilsMessengerEXT(_instance, _dbg_messenger, nullptr);
#endif
    vkDestroyInstance(_instance, nullptr);
    // volk finalize is not necessary
}

// specialization for std::swap

// movable
Instance::Instance(Instance&& other) noexcept {}

Instance& Instance::operator=(Instance&& other) noexcept {}

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

#ifdef ENABLE_VULKAN_VALIDATION
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
#endif
