
#include "./VPhysicalDevice.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "./RhiConfig.h"
#include "./VDevice.h"
#include "Logger.h"
#include "Util.h"

// NOLINTBEGIN
#include "./GraphicsApiCore.h"
// NOLINTEND

using namespace GraphRunner::Rhi;
using namespace GraphRunner::Util;

VPhysicalDevice::VPhysicalDevice( ) : _handle { } {}

VPhysicalDevice::VPhysicalDevice(VPhysicalDevice&& other) noexcept :
    _handle(other._handle) {
    other._handle = VK_NULL_HANDLE;
}

VPhysicalDevice& VPhysicalDevice::operator=(VPhysicalDevice&& other) noexcept {
    if ( this != &other ) {
        std::swap(_handle, other._handle);
    }
    return *this;
}

VPhysicalDevice::~VPhysicalDevice( ) {}

VkPhysicalDevice VPhysicalDevice::handle( ) const {
    return _handle;
}

namespace {

[[nodiscard]]
std::string_view get_physical_device_type_string(VkPhysicalDeviceType type) {
    switch ( type ) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return "Other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "Integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "Discrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "Virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "CPU";
        default:
            return "Unknown";
    }
}

void log_device_properties(VkPhysicalDevice device) {
    // logging device properties
    VkPhysicalDeviceProperties2 prop2 { };
    prop2.pNext = nullptr;
    prop2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    vkGetPhysicalDeviceProperties2(device, &prop2);
    auto const& prop = prop2.properties;

    print_log(
        "Selected Physical Device : {} ({})",
        prop.deviceName,
        get_physical_device_type_string(prop2.properties.deviceType)
    );
    print_log("  nonCoherentAtomSize: {}", prop.limits.nonCoherentAtomSize);
    print_log(
        "  Max UBO size: {} KBytes",
        prop.limits.maxUniformBufferRange / 1024
    );
    print_log(
        "  Max SSBO size: {} KBytes",
        prop.limits.maxStorageBufferRange / 1024
    );
    print_log(
        "  UBO offset alignment: {}",
        prop.limits.minUniformBufferOffsetAlignment
    );
    print_log(
        "  SSBO offset alignment: {}",
        prop.limits.minStorageBufferOffsetAlignment
    );
}

void log_device_features(VkPhysicalDevice device) {
    // logging device features
    VkPhysicalDeviceFeatures2 feat2 { };
    feat2.pNext = nullptr;
    feat2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vkGetPhysicalDeviceFeatures2(device, &feat2);

    print_log("\nDevice Features:");
    print_log(
        "  geometryShader: {}",
        feat2.features.geometryShader ? "YES" : "NO"
    );
    print_log(
        "  tessellationShader: {}",
        feat2.features.tessellationShader ? "YES" : "NO"
    );
}

void log_device_memories(VkPhysicalDevice device) {
    // logging memory properties
    VkPhysicalDeviceMemoryProperties2 mem_prop2 { };
    mem_prop2.pNext = nullptr;
    mem_prop2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    vkGetPhysicalDeviceMemoryProperties2(device, &mem_prop2);
    auto const& prop = mem_prop2.memoryProperties;

    // Print device memory properties
    print_log("\nDevice Memory Properties:");
    print_log("  Memory Type Count: {}", prop.memoryTypeCount);
    for ( uint32_t i = 0; i < prop.memoryTypeCount; ++i ) {
        auto const& memType = prop.memoryTypes[i];
        string propFlags;
        if ( memType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) {
            propFlags += "DEVICE_LOCAL ";
        }
        if ( memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) {
            propFlags += "HOST_VISIBLE ";
        }
        if ( memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) {
            propFlags += "HOST_COHERENT ";
        }
        if ( memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT ) {
            propFlags += "HOST_CACHED ";
        }
        if ( memType.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT ) {
            propFlags += "LAZILY_ALLOCATED ";
        }
        if ( memType.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT ) {
            propFlags += "PROTECTED ";
        }
        if ( propFlags.empty( ) ) {
            propFlags = "NONE";
        } else {
            propFlags.pop_back( ); // Remove trailing space
        }

        print_log(
            "    Memory Type {}: heap {}, flags: {}",
            i,
            memType.heapIndex,
            propFlags
        );

        print_log("  Memory Heap Count: {}", prop.memoryHeapCount);
        for ( uint32_t i = 0; i < prop.memoryHeapCount; ++i ) {
            auto const& heap = prop.memoryHeaps[i];
            string heapFlags;
            if ( heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ) {
                heapFlags += "DEVICE_LOCAL ";
            }
            if ( heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT ) {
                heapFlags += "MULTI_INSTANCE ";
            }
            if ( heapFlags.empty( ) ) {
                heapFlags = "NONE";
            } else {
                heapFlags.pop_back( ); // Remove trailing space
            }

            print_log(
                "    Memory Heap {}: {} MB, flags: {}",
                i,
                heap.size / (1024LL * 1024LL),
                heapFlags
            );
        }
    }
}

void log_device_queue_families(VkPhysicalDevice device) {
    // logging queue family info
    uint32_t qfamily_prop_cnt = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2( // maybe not loaded yet...
        device,
        &qfamily_prop_cnt,
        nullptr
    );
    std::vector<VkQueueFamilyProperties2> que_prop2(
        qfamily_prop_cnt,
        {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2, nullptr, {}}
    );
    vkGetPhysicalDeviceQueueFamilyProperties2(
        device,
        &qfamily_prop_cnt,
        que_prop2.data( )
    );

    // Find queue family properties
    print_log("\nQueue Family Properties: {}", qfamily_prop_cnt);
    for ( size_t i = 0; i < qfamily_prop_cnt; ++i ) {
        auto const& props = que_prop2[i].queueFamilyProperties;
        string queueFlagsStr;
        if ( props.queueFlags & VK_QUEUE_GRAPHICS_BIT ) { // general graphics
            queueFlagsStr += "GRAPHICS ";
        }
        if ( props.queueFlags & VK_QUEUE_COMPUTE_BIT ) { // GPGPU
            queueFlagsStr += "COMPUTE ";
        }
        if ( props.queueFlags & VK_QUEUE_TRANSFER_BIT ) { // data trans only
            queueFlagsStr += "TRANSFER ";
        }
        if ( props.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT ) { // tiling
            queueFlagsStr += "SPARSE_BINDING ";
        } else {
            queueFlagsStr.pop_back( ); // Remove trailing space
        }

        print_log(
            "  Queue Family {}: {} queues, flags: {}",
            i,
            props.queueCount,
            queueFlagsStr
        );
    }
}
} // namespace

void VPhysicalDevice::log_info( ) const {
    log_device_properties(_handle);
    log_device_features(_handle);
    log_device_memories(_handle);
    log_device_queue_families(_handle);
}

namespace {

uint32_t find_queue_family_index(
    std::vector<VkQueueFamilyProperties2> que_family_props,
    VkQueueFlags flags
) {
    // get queue family index
    if ( (flags & VK_QUEUE_TRANSFER_BIT) == flags ) {
        // transfer only
        for ( size_t i = 0; i < que_family_props.size( ); ++i ) {
            auto const& prop = que_family_props[i].queueFamilyProperties;
            if ( (prop.queueFlags & VK_QUEUE_TRANSFER_BIT)
                 && ((prop.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)
                 && ((prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) ) {
                return i;
            }
        }
    } else if ( (flags & VK_QUEUE_COMPUTE_BIT) == flags ) {
        // compute only
        for ( size_t i = 0; i < que_family_props.size( ); ++i ) {
            auto const& prop = que_family_props[i].queueFamilyProperties;
            if ( (prop.queueFlags & VK_QUEUE_COMPUTE_BIT)
                 && ((prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) ) {
                return i;
            }
        }
    } else {
        // else, graphics
        // graphics queue family can handle all 3
        for ( size_t i = 0; i < que_family_props.size( ); ++i ) {
            auto const& prop = que_family_props[i].queueFamilyProperties;
            if ( prop.queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
                return i;
            }
        }
    }

    // unreachable
    return 0;
}

// device queue create info
/*
    문제 1 : 해당 함수를 호출할 때 마다, queue family를 쿼리해오는데, 굉장히 비효율적임.
    문제 2 : queue_cnt 만큼 큐를 가진 queue family가 존재하지 않을 수 있음, 현재는 모자란대로 그대로 감.

    이러한 이유로  add_queue_ci를 재활용하기 위해서는 리팩토링 필요함.
*/
void add_queue_ci_with_family(
    VkPhysicalDevice pdvc,
    VkQueueFlags flags,
    uint32_t queue_cnt,
    std::vector<VkDeviceQueueCreateInfo>& queue_cis,
    std::vector<VkQueueFamilyProperties>& selected_queue_family
) {
    // query queue family properties
    // querying properties per every call can cause waste...
    std::vector<VkQueueFamilyProperties2> que_family_props;
    {
        uint32_t cnt = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(pdvc, &cnt, nullptr);
        que_family_props.resize(
            cnt,
            {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2}
        );
        vkGetPhysicalDeviceQueueFamilyProperties2(
            pdvc,
            &cnt,
            que_family_props.data( )
        );
    }

    VkDeviceQueueCreateInfo que_ci = { };
    float const pq = 1.0f;
    que_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    que_ci.pNext = nullptr;
    que_ci.pQueuePriorities = &pq;
    que_ci.queueFamilyIndex = find_queue_family_index(que_family_props, flags);
    que_ci.queueCount = std::min(
        queue_cnt, // try to get
        que_family_props[que_ci.queueFamilyIndex]
            .queueFamilyProperties.queueCount // actual limit
    );

    // transfer
    queue_cis.push_back(que_ci);
    selected_queue_family.push_back(
        que_family_props[que_ci.queueFamilyIndex].queueFamilyProperties
    );
}

// device extensions
bool set_non_profile_device_extensions(
    VpCapabilities cap,
    VpProfileProperties const& profile,
    std::vector<std::string_view> const& additional_ext_names,
    std::vector<char const*>& non_profile_ext_names
) {
    uint32_t cnt = 0;
    vpGetProfileDeviceExtensionProperties(
        cap,
        &profile,
        nullptr,
        &cnt,
        nullptr
    );
    std::vector<VkExtensionProperties> profile_exts(cnt);
    if ( vpGetProfileDeviceExtensionProperties(
             cap,
             &profile,
             nullptr,
             &cnt,
             profile_exts.data( )
         )
         != VK_SUCCESS ) {
        return false;
    }

    // transform, ext to ext_name(string_view)
    std::set<std::string_view> profile_supported_names;
    std::transform(
        profile_exts.begin( ),
        profile_exts.end( ),
        std::inserter(profile_supported_names, profile_supported_names.end( )),
        [](VkExtensionProperties const& ext_prop) {
            return std::string_view(ext_prop.extensionName);
        }
    );

#ifdef ENABLE_VULKAN_PORTABILITY
    // if portability needed, add portability support extension
    non_profile_ext_names.push_back("VK_KHR_portability_subset");
#endif

    // compare, push back to non_profile_ext_names
    for ( auto const& ext_name : additional_ext_names ) {
        if ( profile_supported_names.find(ext_name)
             == profile_supported_names.end( ) ) {
            // not found -> not in the profile
            non_profile_ext_names.push_back(ext_name.data( ));
        }
    }
    return true;
}

bool check_device_ext_support(
    VkPhysicalDevice pdvc,
    std::vector<char const*> const& final_ext_names
) {
    // profile supported extensions are already checked
    uint32_t cnt = 0;
    vkEnumerateDeviceExtensionProperties(pdvc, nullptr, &cnt, nullptr);
    std::vector<VkExtensionProperties> supported_exts(cnt);
    if ( vkEnumerateDeviceExtensionProperties(
             pdvc,
             nullptr,
             &cnt,
             supported_exts.data( )
         )
         != VK_SUCCESS ) {
        print_log("Querying extension support failed");
        return false;
    }

    for ( auto const& ext : final_ext_names ) {
        // find name among props
        std::string_view ext_name(ext);
        if ( std::find_if(
                 supported_exts.begin( ),
                 supported_exts.end( ),
                 [ext_name](VkExtensionProperties const& ext_prop) {
                     return ext_name
                         == std::string_view(ext_prop.extensionName);
                 }
             )
             == supported_exts.end( ) ) {
            // couldnt find among supported extensions
            return false;
        }
    }
    return true;
}
} // namespace

/*
    단일 graphic queue를 생성하는 device에 대한 factory함수
    baseline profile(작성 기준으론 roadmap_2024)을 기준으로 ext만 추가 가능

    device feature 관련 제어는 전적으로 roadmap_2024에 의존합니다.
    따라서, device 관련 feature를 제어하려면 baseline profile을 교체하세요.
    profile 교체는 RhiConfig.h에서 가능합니다.

    raytracing같은 feature와 연계된 extension의 경우, 별도의 함수를 추후 구현 예정.
*/
std::optional<VDevice>
VPhysicalDevice::create_logical_device_with_single_graphic_queue(
    std::vector<std::string_view> const& ext_names
) const {
    VDevice result;

    // using profile library
    VpCapabilities vp_cap { };
    set_vp_capabilities(vp_cap);
    VpProfileProperties profile {
        RHI_VULKAN_PROFILE_NAME,
        RHI_VULKAN_PROFILE_SPEC_VERSION
    };

    // select queue family to use
    // select single graphic queue
    std::vector<VkDeviceQueueCreateInfo> queue_cis;
    std::vector<VkQueueFamilyProperties> selected_queue_family;
    add_queue_ci_with_family(
        _handle,
        VK_QUEUE_GRAPHICS_BIT,
        1,
        queue_cis,
        selected_queue_family
    );

    // device extension check
    // profile library?
    std::vector<char const*> non_profile_ext_names;
    if ( !set_non_profile_device_extensions(
             vp_cap,
             profile,
             ext_names,
             non_profile_ext_names
         ) ) {
        return std::nullopt;
    }

    // extension support check
    // profile support is already checked
    if ( !check_device_ext_support(_handle, non_profile_ext_names) ) {
        return std::nullopt;
    }

    // create logical device
    VkDeviceCreateInfo device_ci = { };
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.enabledLayerCount = 0; // deprecated
    device_ci.ppEnabledLayerNames = nullptr; // deprecated

    device_ci.queueCreateInfoCount = queue_cis.size( );
    device_ci.pQueueCreateInfos = queue_cis.data( );

    device_ci.enabledExtensionCount = non_profile_ext_names.size( );
    device_ci.ppEnabledExtensionNames = non_profile_ext_names.data( );

    device_ci.pEnabledFeatures = nullptr; // use device feature2 instead
    device_ci.pNext = nullptr; // if there are additinal features, add here

    // init handle
    VpDeviceCreateInfo vp_dev_ci { };
    vp_dev_ci.pCreateInfo = &device_ci;
    vp_dev_ci.enabledFullProfileCount = 1;
    vp_dev_ci.pEnabledFullProfiles = &profile;
    check(vpCreateDevice(vp_cap, _handle, &vp_dev_ci, nullptr, &result._handle)
    );
    // init queue infos
    // save queue family information
    for ( int i = 0; i < queue_cis.size( ); ++i ) {
        result._queue_infos.push_back(
            std::make_tuple(selected_queue_family[i], queue_cis[i], 0)
        );
    }

    // volk load device
    // single device application only
    volkLoadDevice(result._handle);

    return std::move(result);
}
