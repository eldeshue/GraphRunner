
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
    _render_semaphores(nullptr),
    _del_queues( ),
    _del_thread( ) {
    // _del_queus are default constructed, so init device and allocator
    for ( auto& queue : _del_queues ) {
        // Rvalue, must be moved
        queue = DeletionQueue(_rhi_device, _allocator);
    }
    // query device limits
    query_device_limits(_rhi_phys_device, _desc_index_props, _pdv_props);

    // TBD : bindless desc pool
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
// create deletion thread
// timeline semaphores from rendering threads
// must be called before rendering(recording commands)
// param semaphores must not to be reallocated.
void VResourceManager::start_render(std::vector<VkSemaphore> const* semaphores
) {
    // init deletion threads
    if ( _del_thread.joinable( ) ) {
        // end_render was not called, must not happen
        throw std::runtime_error("Error : previous rendering was not ended.");
    }

    // TBD : wait for preloading ends
    // waiting frame index 0 ends

    // set before rendering start
    _render_semaphores = semaphores;

    // init deletion thread
    _del_thread = std::jthread([this](std::stop_token stoken) {
        // exception must stay in the thread
        // or the thread will be terminated
        try {
            uint64_t wait_frame_index =
                1; // frame index 0 means rendering not started
            std::vector<uint64_t> values(this->_render_semaphores->size( ), 0);

            // wait multiple timeline semaphores at the same time
            VkSemaphoreWaitInfo waitInfo { };
            waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            waitInfo.pSemaphores = this->_render_semaphores->data( );
            waitInfo.pValues = values.data( );
            waitInfo.semaphoreCount =
                static_cast<uint32_t>(this->_render_semaphores->size( ));
            waitInfo.flags = 0; // wait for all semaphores
            waitInfo.pNext = nullptr;

            // run until request_stop arrives
            while ( !stoken.stop_requested( ) ) {
                // set frame number to wait
                std::fill(values.begin( ), values.end( ), wait_frame_index);

                // must have time-out to check stop token
                VkResult result = vkWaitSemaphores(
                    this->_rhi_device,
                    &waitInfo,
                    100'000'000 // 100ms
                );

                if ( result == VK_SUCCESS ) {
                    // get bucket index
                    uint32_t bucket_index =
                        wait_frame_index % RHI_MAX_FRAMES_IN_FLIGHT;

                    // delete resources
                    this->_del_queues[bucket_index].deque( );

                    // advance to next frame-index
                    wait_frame_index++;
                } else if ( result == VK_TIMEOUT ) {
                    // GPU is still busy.
                    continue;
                } else {
                    // Device Lost
                    // do not handle, panic
                    // critical error, logging needed
                    print_log(
                        "Error : vkWaitSemaphores failed in deletion thread. failed result is {}.",
                        result
                    );
                    break;
                }
            }
        } catch ( std::exception e ) {
            // logging
            print_log(
                "Error : exception thrown in deletion thread. {}.",
                e.what( )
            );
        }
    });
}

// end rendering
// wait and delete deletion threads
// after thread ends, delete all resources after gpu stops running
// must be called after all rendering ends(gpu sync needed)
void VResourceManager::end_render( ) {
    // clear deletion manager
    {
        // wait for deletion thread ends
        if ( _del_thread.joinable( ) ) {
            _del_thread.request_stop( ); // stop_token 플래그를 true로 설정
            _del_thread.join( ); // 스레드가 루프를 빠져나와 종료될 때까지 대기
        }
        // after join, _del_thread is not joinable

        // after deletion thread ends, manually delete all resources
        for ( auto& queue : _del_queues ) {
            queue.clear_all( );
        }
    }

    // TBD : clear staging manager
}

/* ---------- resource factory ---------- */
namespace {
uint64_t calc_resource_size( ) {
    // calculate size of resource with respect to alignment
}
} // namespace
