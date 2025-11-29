
#include "./DeletionQueue.h"

#include <utility>
#include <variant>

using namespace GraphRunner::Rhi;

DeletionQueue::DeletionQueue( ) :
    _queue( ),
    _local_deletion_queue( ),
    _mtx( ),
    _device(VK_NULL_HANDLE),
    _allocator(VK_NULL_HANDLE) {}

DeletionQueue::DeletionQueue(VkDevice device, VmaAllocator allocator) :
    _queue( ),
    _local_deletion_queue( ),
    _mtx( ),
    _device(device),
    _allocator(allocator) {}

DeletionQueue::~DeletionQueue( ) {
    // do not own vma allocator
    clear_all( );
}

DeletionQueue::DeletionQueue(DeletionQueue&& other) noexcept :
    _queue(std::move(other._queue)),
    _local_deletion_queue(std::move(other._local_deletion_queue)),
    _mtx( ),
    _device(other._device),
    _allocator(other._allocator) {
    // other.queue is lvalue, so std::move needed
    other._device = VK_NULL_HANDLE;
    other._allocator = VK_NULL_HANDLE;
}

DeletionQueue& DeletionQueue::operator=(DeletionQueue&& other) noexcept {
    if ( this != &other ) {
        _queue.swap(other._queue);
        _local_deletion_queue.swap(other._local_deletion_queue);
        // cannot move mutex
        std::swap(_device, other._device);
        std::swap(_allocator, other._allocator);
    }
    return *this;
}

void DeletionQueue::enque(DeletionObj obj) {
    {
        // lock
        std::scoped_lock lock(_mtx);
        // push back
        _queue.push_back(obj);
        // unlock
    }
}

namespace {
using DeletionObj = DeletionQueue::DeletionObj;
using VmaImage = DeletionQueue::VmaImage;
using VmaBuffer = DeletionQueue::VmaBuffer;

void process_deletions(
    VkDevice device,
    VmaAllocator allocator,
    std::vector<DeletionObj>& queue
) {
    for ( auto const& obj : queue ) {
        // by using std::visit, proper deletion can be called in constant time
        std::visit(
            [&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr ( std::is_same_v<T, VmaImage> ) {
                    // VmaImage = { VmaAllocation, VkImage}
                    vmaDestroyImage(allocator, arg.second, arg.first);
                } else if constexpr ( std::is_same_v<T, VmaBuffer> ) {
                    // VmaBuffer = { VmaAllocation, VkBuffer}
                    vmaDestroyBuffer(allocator, arg.second, arg.first);
                } else if constexpr ( std::is_same_v<T, VkImageView> ) {
                    vkDestroyImageView(device, arg, nullptr);
                } else if constexpr ( std::is_same_v<T, VkSampler> ) {
                    vkDestroySampler(device, arg, nullptr);
                } else if constexpr ( std::is_same_v<T, VkFramebuffer> ) {
                    vkDestroyFramebuffer(device, arg, nullptr);
                } else {
                    // unreachable
                }
            },
            obj
        );
    }

    // set size to zero
    // keep capacity
    queue.clear( );
}
} // namespace

/**
 * @brief 큐에 저장된 데이터를 소비한다.
 * @detail
 * 렌더링 중 호출되어 gpu 동기화 된 deletion을 수행함.
 * timeline semaphore에 의해 gpu의 프레임 종료 signal에 동기화 필요. 
 * 
 * allocation을 최소화 하기 위해서 queue를 swap한다. 
 * queue의 swap으로 인해서 enque와 병렬로 실행 가능하다. 
 * 
 * @warning 이 함수를 호출할 deletion thread는 유일해야 한다.
 * 그 이유는 내부적으로 사용하는 _local_deletion_queue가 동기화되지 않기 때문.
 */
void DeletionQueue::deque( ) {
    {
        // lock
        std::scoped_lock lock(_mtx);
        if ( _queue.empty( ) ) {
            return;
        }
        // swap the vector
        this->_queue.swap(_local_deletion_queue);
        // unlock
    }

    // consume data in local deletion queue
    process_deletions(_device, _allocator, _local_deletion_queue);
}

/**
 * @brief 큐에 저장된 데이터를 모두 소비한다.
 * @detail
 * 객체가 보유하는 두 벡터의 모든 원소에 대해서 소멸을 수행한다.
 * 렌더링이 종료된 후 호출하는 것을 상정한다(즉, 동기화 불필요).
 * ex) end of level
 * 
 * @warning 이 함수를 호출할 deletion thread는 유일해야 한다.
 * 렌더링 중 호출될 deque 및 enque와 동시에 호출될 경우 문제가 발생한다.
 * 반드시 렌더링 종료 후 호출되어야 한다.
 */
void DeletionQueue::clear_all( ) {
    // lock
    std::scoped_lock lock(_mtx);
    process_deletions(_device, _allocator, _queue);
    process_deletions(_device, _allocator, _local_deletion_queue);
    // unlock
}
