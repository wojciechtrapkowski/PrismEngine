#include "resources/vulkan/vk_texture_resource.hpp"

namespace Prism::Resources
{
    VkTextureResource::VkTextureResource(VkDevice device, Resources::VkImageResource imageResource, VkImageView imageView, VkSampler sampler) :
        _device(device), _imageResource(std::move(imageResource)), _imageView(imageView), _sampler(sampler)
    {}

    VkTextureResource::~VkTextureResource()
    {
        if (_device == VK_NULL_HANDLE) {
            return;
        }

        if (_sampler != VK_NULL_HANDLE) {
            vkDestroySampler(_device, _sampler, nullptr);
        }
        if (_imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(_device, _imageView, nullptr);
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

        swap(first._device, second._device);
        swap(first._imageResource, second._imageResource);
        swap(first._imageView, second._imageView);
        swap(first._sampler, second._sampler);
    }
} // namespace Prism::Resources