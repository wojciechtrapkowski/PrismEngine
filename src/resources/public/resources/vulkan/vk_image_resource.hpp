#pragma once

#include "resources/resource.hpp"

#include "volk/volk.h"
#include "vk_mem_alloc.h"

namespace Prism::Resources
{
    struct VkImageResource : ResourceImpl<VkImageResource>
    {
        VkImageResource() = default;
        VkImageResource(VmaAllocator allocator, VmaAllocation allocation, VkImage image);
        ~VkImageResource();

        VkImageResource(VkImageResource&& other) noexcept;

        VkImageResource& operator=(VkImageResource&& other) noexcept;

        VkImageResource(const VkImageResource&)            = delete;
        VkImageResource& operator=(const VkImageResource&) = delete;

        VmaAllocation GetAllocation() const { return _allocation; }
        VkImage       GetImage() const { return _image; }

    private:
        friend void swap(VkImageResource& first, VkImageResource& second) noexcept;

        VmaAllocator  _allocator  = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;
        VkImage       _image      = VK_NULL_HANDLE;
    };
} // namespace Prism::Resources