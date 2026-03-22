#include "resources/vulkan/vk_staging_buffer_resource.hpp"

#include <stdexcept>
#include <utility>

namespace Prism::Resources
{
    VkStagingBufferResource::VkStagingBufferResource(VmaAllocator allocator) : _allocator(allocator)
    {
        _stagingBuffer =
            VkBufferResource<>(_allocator, INITIAL_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        _currentlyUtilized = 0;
    }

    VkStagingBufferResource::VkStagingBufferResource(VkStagingBufferResource&& other) noexcept
    {
        swap(*this, other);
    }

    VkStagingBufferResource& VkStagingBufferResource::operator=(VkStagingBufferResource&& other) noexcept
    {
        if (this != &other) {
            swap(*this, other);
        }
        return *this;
    }

    void VkStagingBufferResource::Copy(VkBuffer destination, void* data, size_t size, size_t offset)
    {
        checkIfResizeIsNeeded(size);

        void* mappedData;
        vmaMapMemory(_allocator, _stagingBuffer.GetAllocation(), &mappedData);
        std::memcpy(static_cast<char*>(mappedData) + _currentlyUtilized, data, size);
        vmaUnmapMemory(_allocator, _stagingBuffer.GetAllocation());

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = _currentlyUtilized;
        copyRegion.dstOffset = offset;
        copyRegion.size      = size;

        _pendingCopies.push_back({destination, copyRegion});

        _currentlyUtilized += size;
    }

    void VkStagingBufferResource::CopyToImage(VkImage destination, void* data, size_t size, uint32_t width, uint32_t height)
    {
        checkIfResizeIsNeeded(size);

        void* mappedData;
        vmaMapMemory(_allocator, _stagingBuffer.GetAllocation(), &mappedData);
        std::memcpy(static_cast<char*>(mappedData) + _currentlyUtilized, data, size);
        vmaUnmapMemory(_allocator, _stagingBuffer.GetAllocation());

        VkBufferImageCopy region{};
        region.bufferOffset                    = _currentlyUtilized;
        region.bufferRowLength                 = 0;
        region.bufferImageHeight               = 0;
        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = 1;
        region.imageOffset                     = {0, 0, 0};
        region.imageExtent                     = {width, height, 1};

        _pendingImageCopies.push_back({destination, region});
        _currentlyUtilized += size;
    }

    void VkStagingBufferResource::CopyImmediately(VkCommandBuffer commandBuffer, VmaAllocation destinationAllocation, void* data, size_t size)
    {
        void* mappedData;
        vmaMapMemory(_allocator, destinationAllocation, &mappedData);
        std::memcpy(static_cast<char*>(mappedData), data, size);
        vmaUnmapMemory(_allocator, destinationAllocation);
        vmaFlushAllocation(_allocator, destinationAllocation, 0, size);
    }

    void VkStagingBufferResource::Commit(VkCommandBuffer commandBuffer)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        if (_pendingCopies.empty()) {
            vkEndCommandBuffer(commandBuffer);
            return;
        }

        for (const auto& pending : _pendingCopies) {
            vkCmdCopyBuffer(commandBuffer, _stagingBuffer.GetBuffer(), pending.destination, 1, &pending.region);
        }

        for (const auto& pending : _pendingImageCopies) {
            vkCmdCopyBufferToImage(commandBuffer, _stagingBuffer.GetBuffer(), pending.destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &pending.region);
        }

        _pendingCopies.clear();
        _pendingImageCopies.clear();
        _currentlyUtilized = 0;

        vkEndCommandBuffer(commandBuffer);
    }

    void VkStagingBufferResource::checkIfResizeIsNeeded(size_t additionalSize)
    {
        if (additionalSize + _currentlyUtilized > _stagingBuffer.GetBufferSize()) {
            size_t newSize = std::max(_stagingBuffer.GetBufferSize() * 2, static_cast<VkDeviceSize>(additionalSize + _currentlyUtilized));

            VkBufferResource<> newBuffer(_allocator, newSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

            // Copy old contents & replace previous buffer with the new one.
            if (_currentlyUtilized > 0) {
                void* oldData = nullptr;
                vmaMapMemory(_allocator, _stagingBuffer.GetAllocation(), &oldData);

                void* newData = nullptr;
                vmaMapMemory(_allocator, newBuffer.GetAllocation(), &newData);

                std::memcpy(newData, oldData, _currentlyUtilized);

                vmaUnmapMemory(_allocator, newBuffer.GetAllocation());
                vmaUnmapMemory(_allocator, _stagingBuffer.GetAllocation());
            }

            _stagingBuffer = std::move(newBuffer);
        }
    }

    void swap(VkStagingBufferResource& first, VkStagingBufferResource& second) noexcept
    {
        using std::swap;
        swap(first._allocator, second._allocator);
        swap(first._stagingBuffer, second._stagingBuffer);
        swap(first._currentlyUtilized, second._currentlyUtilized);
        swap(first._pendingCopies, second._pendingCopies);
        swap(first._pendingImageCopies, second._pendingImageCopies);
    }
} // namespace Prism::Resources