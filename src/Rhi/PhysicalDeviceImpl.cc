
#include "./PhysicalDeviceImpl.h"

#include <string_view>
#include <vector>

#include "Logger.h"
#include "Volk/volk.h"

using namespace GraphRunner::Rhi::Impl;
using namespace GraphRunner::Util;

PhysicalDeviceImpl::PhysicalDeviceImpl( ) : _pdvc { } {}

PhysicalDeviceImpl::~PhysicalDeviceImpl( ) {}

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

    // 선택한 gpu 성능 출력
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
                heap.size / (1024 * 1024),
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
    // gpu의 커맨드 프로세서를 추상화 한 것이 command queue
    // gpu에는 용도에 따라 여러 큐로 추상화 되었음.
    // 일반 그래픽스 명령 큐, 컴퓨트 큐, 전송, 타일 스트리밍 등 용도가 있음.
    // 항상 큐패밀리에서 쿼리해서 큐를 생성하게 됨
    print_log("\nQueue Family Properties: {}", qfamily_prop_cnt);
    for ( size_t i = 0; i < qfamily_prop_cnt; ++i ) {
        auto const& props = que_prop2[i].queueFamilyProperties;
        string queueFlagsStr;
        if ( props.queueFlags & VK_QUEUE_GRAPHICS_BIT ) { // 범용 그래픽스
            queueFlagsStr += "GRAPHICS ";
        }
        if ( props.queueFlags & VK_QUEUE_COMPUTE_BIT ) { // GPGPU
            queueFlagsStr += "COMPUTE ";
        }
        if ( props.queueFlags & VK_QUEUE_TRANSFER_BIT ) { // 데이터 전송 only
            queueFlagsStr += "TRANSFER ";
        }
        if ( props.queueFlags
             & VK_QUEUE_SPARSE_BINDING_BIT ) { // tiling 수행 용도
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

void PhysicalDeviceImpl::log_info( ) const {
    log_device_properties(_pdvc);
    log_device_features(_pdvc);
    log_device_memories(_pdvc);
    log_device_queue_families(_pdvc);
}
