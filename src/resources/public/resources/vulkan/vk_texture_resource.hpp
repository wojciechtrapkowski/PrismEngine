#pragma once

#include "resources/resource.hpp"

#include "volk/volk.h"
#include "vk_mem_alloc.h"

namespace Prism::Resources
{
    struct VkTextureResource : ResourceImpl<VkTextureResource>
    {
        VkTextureResource(VkDevice device, VkImage image, VkImageView imageView, VkSampler sampler);
        ~VkTextureResource() = default;

        VkTextureResource(VkTextureResource&& other) noexcept;

        VkTextureResource& operator=(VkTextureResource&& other) noexcept;

        VkTextureResource(const VkTextureResource&)            = delete;
        VkTextureResource& operator=(const VkTextureResource&) = delete;

    private:
        friend void swap(VkTextureResource& first, VkTextureResource& second) noexcept;

        VkDevice    device    = VK_NULL_HANDLE;
        VkImage     image     = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        VkSampler   sampler   = VK_NULL_HANDLE;
    };
} // namespace Prism::Resources