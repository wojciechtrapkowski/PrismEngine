#include "resources/vulkan/vk_texture_resource.hpp"

namespace Prism::Resources
{
    VkTextureResource::VkTextureResource(VkDevice device, VkImage image, VkImageView imageView, VkSampler sampler) :
        device(device), image(image), imageView(imageView), sampler(sampler)
    {}

    VkTextureResource::~VkTextureResource()
    {
        if (device == VK_NULL_HANDLE) {
            return;
        }

        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device, sampler, nullptr);
        }
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, imageView, nullptr);
        }
        if (image != VK_NULL_HANDLE) {
            vkDestroyImage(device, image, nullptr);
        }
    }

    VkTextureResource::VkTextureResource(VkTextureResource&& other) noexcept
    {
        swap(*this, other);
    }

    VkTextureResource& VkTextureResource::operator=(VkTextureResource&& other) noexcept
    {
        if (this != &other) {
            swap(*this, other);
        }
        return *this;
    }

    void swap(VkTextureResource& first, VkTextureResource& second) noexcept
    {
        using std::swap;

        swap(first.device, second.device);
        swap(first.image, second.image);
        swap(first.imageView, second.imageView);
        swap(first.sampler, second.sampler);
    }
} // namespace Prism::Resources