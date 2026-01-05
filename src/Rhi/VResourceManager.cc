
#include "./VResourceManager.h"

#include <Util.h>

#include "./RhiConfig.h"

using namespace GraphRunner::Rhi;
using namespace GraphRunner::Util;

namespace {
VmaAllocator
create_vma_with_volk(VkInstance inst, VkPhysicalDevice pdvc, VkDevice dvc) {
    VmaAllocator result {VK_NULL_HANDLE};

    // vma initializatioin
    VmaAllocatorCreateInfo vma_ci = { };
    vma_ci.instance = inst;
    vma_ci.physicalDevice = pdvc;
    vma_ci.device = dvc;
    vma_ci.vulkanApiVersion = RHI_VULKAN_API_VERSION;
    // activated device features(extensions)
    // below features are activated in VRhi
    vma_ci.flags = VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT
        | VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT
        | VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT
        | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    // load functions from volk
    VmaVulkanFunctions functions = { };
    check(vmaImportVulkanFunctionsFromVolk(&vma_ci, &functions));
    vma_ci.pVulkanFunctions = &functions;

    // init allocator
    check(vmaCreateAllocator(&vma_ci, &result));

    return result;
}

void query_device_limits(
    VkPhysicalDevice physical_device,
    VkPhysicalDeviceDescriptorIndexingProperties& indexing_props,
    VkPhysicalDeviceProperties2& props2
) {
    // Vulkan 1.2 core or VK_EXT_descriptor_indexing needed
    // current base profile is vulkan 1.3+
    indexing_props.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
    indexing_props.pNext = nullptr;

    // general properties
    props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    props2.pNext = &indexing_props; // chaining

    // query data
    vkGetPhysicalDeviceProperties2(physical_device, &props2);
}

// default staging heap size, 64MB
constexpr VkDeviceSize DEFAULT_STAGING_HEAP_SIZE =
    static_cast<VkDeviceSize>(RHI_DEFAULT_STAGING_HEAP_SIZE_MB) * 1024 * 1024;
} // namespace

// basic creation method
// need reference of flight frame number, ref to frame index, vk device
// maybe another queue for transfer
VResourceManager::VResourceManager(
    std::atomic_uint64_t& frame_idx,
    VkInstance inst,
    VkPhysicalDevice pdvc,
    VkDevice dvc
) :
    _rhi_frame_index(frame_idx),
    _rhi_device(dvc),
    _rhi_phys_device(pdvc),
    _allocator(create_vma_with_volk(inst, pdvc, dvc)),
    _del_queues( ) {
    // query device limits
    query_device_limits(_rhi_phys_device, _desc_index_props, _pdv_props);

    // push per frame resources
    for ( int i = 0; i < RHI_MAX_FRAMES_IN_FLIGHT; ++i ) {
        // _del_queus
        _del_queues.push_back(DeletionQueue(_rhi_device, _allocator));
        // staging buffers
        _staging_heaps.push_back(VStagingHeap(
            *this,
            _pdv_props.properties.limits.nonCoherentAtomSize,
            DEFAULT_STAGING_HEAP_SIZE // 64MB
        ));
    }

    // TBD : bindless desc manager
    // use device limit
}

VResourceManager::~VResourceManager( ) {
    // clean up resources
    end_render( );
    // destroy vma allocator
    if ( _allocator != VK_NULL_HANDLE ) {
        vmaDestroyAllocator(_allocator);
    }
}

/* ---------- resource control ---------- */
namespace {
// helper functions
}

// start rendering
// wait for preload ends
// must be called before rendering(recording commands)
// param semaphores must not to be reallocated.
void VResourceManager::start_render( ) {
    // TBD : call routine that must be called before rendering starts
}

// end rendering
// must be called to end rendering(after gpu synced)
void VResourceManager::end_render( ) {
    // clear deletion manager
    {
        // after deletion thread ends, manually delete all resources
        for ( auto& queue : _del_queues ) {
            queue.clear_all( );
        }
    }

    // TBD : clear staging queue
}

/* ---------- frame based resource control ---------- */
// start frame
// must be called after the frame index updated
// delete all data in current frame's deletion queue
void VResourceManager::start_frame( ) {
    // delete all data in current deletion queue
    // those are ready to be deleted in gpu
    get_current_deletion_queue( ).clear_all( );

    // TBD : per frame, before render starts
}

// end frame
// must be called before the next frame starts
void VResourceManager::end_frame( ) {
    // TBD : per frame, before next render starts
}

/* ---------- resource factory ---------- */
namespace {} // namespace
