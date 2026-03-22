#pragma once

#include "resources/resource.hpp"

#include "volk/volk.h"
#include "vk_mem_alloc.h"

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Prism::Resources
{
    namespace
    {
        auto getBufferDeviceAddress = [](VkDevice device, VkBuffer buffer) {
            VkBufferDeviceAddressInfoKHR addressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR};
            addressInfo.buffer = buffer;
            return vkGetBufferDeviceAddress(device, &addressInfo);
        };
    } // namespace

    template<typename T = void>
    struct VkBufferResource : ResourceImpl<VkBufferResource<T>>
    {
    private:
        VkBuffer      buffer     = VK_NULL_HANDLE;
        VkDeviceSize  bufferSize = 0;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VmaAllocator  allocator  = VK_NULL_HANDLE;

    public:
        VkBufferResource() = default;

        explicit VkBufferResource(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO) :
            bufferSize(size), allocator(allocator)
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size        = size;
            bufferInfo.usage       = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = memoryUsage;

            if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
                buffer     = VK_NULL_HANDLE;
                allocation = VK_NULL_HANDLE;
                throw std::runtime_error("Failed to create Vulkan buffer!");
            }
        }

        explicit VkBufferResource(
            VmaAllocator       allocator,
            VkDeviceSize       size,
            VkBufferUsageFlags usage,
            VkDeviceSize       minAlignment,
            VmaMemoryUsage     memoryUsage = VMA_MEMORY_USAGE_AUTO) : bufferSize(size), allocator(allocator)
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size        = size;
            bufferInfo.usage       = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = memoryUsage;

            if (vmaCreateBufferWithAlignment(allocator, &bufferInfo, &allocInfo, minAlignment, &buffer, &allocation, nullptr) != VK_SUCCESS) {
                buffer     = VK_NULL_HANDLE;
                allocation = VK_NULL_HANDLE;
                throw std::runtime_error("Failed to create aligned Vulkan buffer!");
            }
        }

        ~VkBufferResource()
        {
            if (buffer != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE && allocator != VK_NULL_HANDLE) {
                vmaDestroyBuffer(allocator, buffer, allocation);
            }
        }

        friend void swap(VkBufferResource<T>& first, VkBufferResource<T>& second) noexcept
        {
            using std::swap;
            swap(first.buffer, second.buffer);
            swap(first.bufferSize, second.bufferSize);
            swap(first.allocation, second.allocation);
            swap(first.allocator, second.allocator);
        }

        VkBufferResource(VkBufferResource&& other) noexcept { swap(*this, other); }

        VkBufferResource& operator=(VkBufferResource&& other) noexcept
        {
            if (this != &other) {
                swap(*this, other);
            }
            return *this;
        }

        VkBufferResource(const VkBufferResource&)            = delete;
        VkBufferResource& operator=(const VkBufferResource&) = delete;

        VkBuffer GetBuffer() const { return buffer; }

        VkDeviceAddress GetBufferDeviceAddress() const
        {
            if (buffer == VK_NULL_HANDLE) {
                throw std::runtime_error("Buffer is not initialized!");
            }

            VmaAllocatorInfo allocatorInfo{};
            vmaGetAllocatorInfo(allocator, &allocatorInfo);
            VkDevice device = allocatorInfo.device;

            return getBufferDeviceAddress(device, buffer);
        }

        VmaAllocation GetAllocation() const { return allocation; }

        VkDeviceSize GetBufferSize() const { return bufferSize; }

        constexpr VkDeviceSize GetElementSize() const
            requires(!std::is_void_v<T>)
        {
            return static_cast<VkDeviceSize>(sizeof(T));
        }

        VkDeviceSize GetElementCount() const
            requires(!std::is_void_v<T>)
        {
            return bufferSize / static_cast<VkDeviceSize>(sizeof(T));
        }
    };
} // namespace Prism::Resources