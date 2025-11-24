
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
    _allocator(create_vma_with_volk(inst, pdvc, dvc)),
    _semaphores(nullptr) {
    // init
}

VResourceManager::~VResourceManager( ) {
    // destroy vma allocator
    if ( _allocator != VK_NULL_HANDLE ) {
        vmaDestroyAllocator(_allocator);
    }
}

void VResourceManager::set_semaphores(std::vector<VkSemaphore>* semaphores) {
    // set before rendering start
    _semaphores = semaphores;
}

namespace {
uint64_t calc_resource_size( ) {
    // calculate size of resource with respect to alignment
}
} // namespace
