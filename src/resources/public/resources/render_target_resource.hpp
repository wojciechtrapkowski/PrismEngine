#pragma once

#include "resources/resource.hpp"

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include <optional>

namespace Prism::Resources
{
    struct RenderTargetResource : ResourceImpl<RenderTargetResource>
    {
        enum RenderTargetCreationFlags
        {
            COLOR_ATTACHMENT         = 1u << 0,
            DEPTH_STENCIL_ATTACHMENT = 1u << 1,
        };

        RenderTargetResource(VkDevice device, VmaAllocator allocator, VkExtent2D extent, uint32_t flags);
        ~RenderTargetResource();

        RenderTargetResource(const RenderTargetResource&)            = delete;
        RenderTargetResource& operator=(const RenderTargetResource&) = delete;

        RenderTargetResource(RenderTargetResource&&) noexcept;

        RenderTargetResource& operator=(RenderTargetResource&&) noexcept;

        VkImage GetColorImage() const;

        VkImageView GetColorImageView() const;

        VkImage GetDepthImage() const;

        VkImageView GetDepthImageView() const;

        static VkFormat GetDepthFormat() { return DEPTH_FORMAT; }

        static VkFormat GetColorFormat() { return COLOR_FORMAT; }

    private:
        // For now, maybe in the future parametrize it. TODO: Move it to VulkanResource
        static const VkFormat              COLOR_FORMAT      = VK_FORMAT_R8G8B8A8_UNORM;
        static const VkFormat              DEPTH_FORMAT      = VK_FORMAT_D32_SFLOAT_S8_UINT;
        VkSampleCountFlagBits NUMBER_OF_SAMPLES = VK_SAMPLE_COUNT_1_BIT;

        VkDevice     device    = VK_NULL_HANDLE;
        VmaAllocator allocator = VK_NULL_HANDLE;
        VkExtent2D   extent    = {0, 0};

        VkImage       colorImage           = VK_NULL_HANDLE;
        VmaAllocation colorImageAllocation = VK_NULL_HANDLE;
        VkImageView   colorImageView       = VK_NULL_HANDLE;

        VkImage       depthImage           = VK_NULL_HANDLE;
        VmaAllocation depthImageAllocation = VK_NULL_HANDLE;
        VkImageView   depthImageView       = VK_NULL_HANDLE;

        friend void swap(RenderTargetResource& a, RenderTargetResource& b) noexcept;
    };
} // namespace Prism::Resources