#pragma once

#include "resources/resource.hpp"
#include "resources/vulkan/vk_buffer_resource.hpp"

#include "vulkan/vulkan.h"

namespace Prism::Resources
{
    struct VkAccelerationStructureResource : ResourceImpl<VkAccelerationStructureResource>
    {
        VkAccelerationStructureResource(
            VkDevice                      device,
            Resources::VkBufferResource<> accelStructBuffer,
            Resources::VkBufferResource<> scratchBuffer,
            VkAccelerationStructureKHR    accelStruct);
        ~VkAccelerationStructureResource();

        VkAccelerationStructureResource(VkAccelerationStructureResource& other)            = delete;
        VkAccelerationStructureResource& operator=(VkAccelerationStructureResource& other) = delete;

        VkAccelerationStructureResource(VkAccelerationStructureResource&& other) noexcept;
        VkAccelerationStructureResource& operator=(VkAccelerationStructureResource&& other) noexcept;

        VkAccelerationStructureKHR GetAccelerationStructure() const { return _accelStruct; }

        Resources::VkBufferResource<>& GetAccelerationStructureBuffer() { return _accelStructBuffer; }

        void DestroyScratchBuffer();

    private:
        friend void swap(VkAccelerationStructureResource& lhs, VkAccelerationStructureResource& rhs) noexcept;

        VkDevice                   _device      = {};
        VkAccelerationStructureKHR _accelStruct = VK_NULL_HANDLE;

        Resources::VkBufferResource<> _accelStructBuffer = {};
        Resources::VkBufferResource<> _scratchBuffer     = {};
    };
} // namespace Prism::Resources
