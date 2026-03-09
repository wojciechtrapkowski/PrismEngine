#include "resources/vulkan/vk_acceleration_structure_resource.hpp"

namespace Prism::Resources
{
    VkAccelerationStructureResource::VkAccelerationStructureResource(
        VkDevice device, Resources::VkBufferResource<> accelStructBuffer, Resources::VkBufferResource<> scratchBuffer, VkAccelerationStructureKHR accelStruct) :
        _device(device),
        _accelStructBuffer(std::move(accelStructBuffer)), _scratchBuffer(std::move(scratchBuffer)), _accelStruct(std::move(accelStruct))
    {}

    VkAccelerationStructureResource::~VkAccelerationStructureResource()
    {
        if (_accelStruct == VK_NULL_HANDLE) {
            return;
        }

        vkDestroyAccelerationStructureKHR(_device, _accelStruct, nullptr);

        _accelStruct = VK_NULL_HANDLE;
    }

    VkAccelerationStructureResource::VkAccelerationStructureResource(VkAccelerationStructureResource&& other) noexcept
    {
        swap(*this, other);
    }

    VkAccelerationStructureResource& VkAccelerationStructureResource::operator=(VkAccelerationStructureResource&& other) noexcept
    {
        swap(*this, other);
        return *this;
    }

    void VkAccelerationStructureResource::DestroyScratchBuffer()
    {
        _scratchBuffer = {};
    }

    void swap(VkAccelerationStructureResource& lhs, VkAccelerationStructureResource& rhs) noexcept
    {
        using std::swap;

        swap(lhs._device, rhs._device);
        swap(lhs._accelStruct, rhs._accelStruct);
        swap(lhs._accelStructBuffer, rhs._accelStructBuffer);
        swap(lhs._scratchBuffer, rhs._scratchBuffer);
    }
} // namespace Prism::Resources