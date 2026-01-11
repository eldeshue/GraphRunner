
#include <deque>
#include <set>
#include <variant>
#include <vector>

#include "./Util.h"
#include "GraphicsApiCore.h"
#include "RenderSystem.h"

namespace GraphRunner {
namespace Rhi {
    class VResourceManager;
    class VStagingHeap;
    class VBuffer;
    class VImage;
    class VRhi;

    struct VSyncBufferTransferInfo {
        VBuffer* dst;
        VkDeviceSize dst_offset;
    };

    struct VSyncImageTransferInfo {
        VImage* dst;
        VkOffset3D dst_offset;
        VkImageSubresourceLayers dst_range;
    };

    struct VSyncTransferInfo {
        std::variant<VSyncBufferTransferInfo, VSyncImageTransferInfo>
            target_info;
        // copy source info
        void* psrc;
        VkDeviceSize size;
        VkDeviceSize alignment;
    };

    class VSyncTransferSystem: public RenderSystem {
      private:
        // no copy
        VSyncTransferSystem(VSyncTransferSystem const&) = delete;
        VSyncTransferSystem& operator=(VSyncTransferSystem const&) = delete;

        // barrier holder
        // all barrier will be collected and consumed during flush
        std::vector<VkBufferMemoryBarrier2> _before_flush_buf_barriers;
        std::vector<VkImageMemoryBarrier2> _before_flush_img_barriers;
        std::vector<VkBufferMemoryBarrier2> _after_flush_buf_barriers;
        std::vector<VkImageMemoryBarrier2> _after_flush_img_barriers;

        // per frame mapped buffer for staging(host visible, host coherent)
        std::vector<VStagingHeap> _staging_heaps;

        struct SyncTransferQueue {
            // buffer to buffer copy vector
            std::set<VBuffer*> _buf_target; // to prevent double check
            std::deque<VkBufferCopy2>
                _buf_copy_rgns; // use deque to prevent reallocation
            std::vector<VkCopyBufferInfo2> _buf_cpy_infos;
            // buffer to image copy vector
            std::set<VImage*> _img_target; // to prevent double check
            std::deque<VkBufferImageCopy2>
                _img_copy_rgns; // to prevent reallocation
            std::vector<VkCopyBufferToImageInfo2> _img_cpy_infos;
        };

        std::vector<SyncTransferQueue> _sync_transfer_queues;

      public:
        // take pointer to rhi, need Resource manager
        VSyncTransferSystem(VRhi* prhi);

        ~VSyncTransferSystem( ) {}

        VSyncTransferSystem(VSyncTransferSystem&& other) noexcept;
        VSyncTransferSystem& operator=(VSyncTransferSystem&& other) noexcept;

        /* ---------- public interface ---------- */
        // after frame start, reset the offset
        // TBD : Bindless manager 구현 후, 저장된 bindless 요청을 전부 읽어오는 구현 필요
        void reset(uint32_t cur_frame_index);

        // get copy request dispatched by RDG
        virtual void execute(uint32_t cur_frame_index, void* param);

        // TBD : implement COPY_PASS for RDG
        // transfer manager는 나중에 RDG에 copy pass를 제공해야 함.
        // 현재는 RDG 설계가 안되어 있으므로, 추후 RDG 설계와 함께 구현 추가 필요
        // 구현 시점에 flush 부분을 참고해서 barrier 자동 삽입 로직 및 copy 구현

        /* ---------- helper functions ---------- */
        // record all copy command without RDG
        // using pipeline barrier for sync
        // for simple usage
        void flush(VkCommandBuffer cmd_buffer, uint32_t cur_frame_index);
    };
} // namespace Rhi
} // namespace GraphRunner
