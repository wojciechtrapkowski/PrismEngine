#pragma once

#include "resources/resource.hpp"
#include "resources/vulkan/vk_buffer_resource.hpp"

#include "volk/volk.h"
#include "vk_mem_alloc.h"

#include <vector>

namespace Prism::Resources
{
    struct PendingCopy
    {
        VkBuffer     destination;
        VkBufferCopy region;
    };

    struct PendingImageCopy
    {
        VkImage           destination;
        VkBufferImageCopy region;

        // VkImageMemoryBarrier before
        // VkImageMemoryBarrier after
        // We could create a resource, that will also contain src stage and dst stage
    };

    struct VkStagingBufferResource : ResourceImpl<VkStagingBufferResource>
    {
        VkStagingBufferResource(VmaAllocator allocator);
        ~VkStagingBufferResource() = default;

        VkStagingBufferResource(VkStagingBufferResource&& other) noexcept;

        VkStagingBufferResource& operator=(VkStagingBufferResource&& other) noexcept;

        VkStagingBufferResource(const VkStagingBufferResource&)            = delete;
        VkStagingBufferResource& operator=(const VkStagingBufferResource&) = delete;

        void Copy(VkBuffer destination, void* data, size_t size, size_t offset = 0);

        void CopyToImage(VkImage destination, void* data, size_t size, uint32_t width, uint32_t height);

        void CopyImmediately(VkCommandBuffer commandBuffer, VmaAllocation destinationAllocation, void* data, size_t size);

        void Commit(VkCommandBuffer commandBuffer);

    private:
        void checkIfResizeIsNeeded(size_t additionalSize = 0);

        static constexpr const VkDeviceSize INITIAL_SIZE = 10000;
        friend void                         swap(VkStagingBufferResource& first, VkStagingBufferResource& second) noexcept;

        VmaAllocator _allocator = VK_NULL_HANDLE;

        Resources::VkBufferResource<> _stagingBuffer     = {};
        size_t                        _currentlyUtilized = 0;

        std::vector<PendingCopy>      _pendingCopies      = {};
        std::vector<PendingImageCopy> _pendingImageCopies = {};
    };
} // namespace Prism::Resources