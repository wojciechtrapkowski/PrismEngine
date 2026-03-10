#pragma once

#include "resources/vulkan_resource.hpp"
#include "resources/scene.hpp"
#include "resources/context_resources.hpp"
#include "resources/render_target_resource.hpp"
#include "resources/vulkan/vk_staging_buffer_resource.hpp"
#include "resources/vulkan/vk_acceleration_structure_resource.hpp"

#include "components/transform.hpp"

#include "volk/volk.h"

#include <vector>

namespace Prism::Systems::Subsystems::MeshDrawingSystem
{
    struct RaytracedGeometryDrawingSubsystem
    {
    public:
        RaytracedGeometryDrawingSubsystem(Resources::ContextResources& contextResources);
        ~RaytracedGeometryDrawingSubsystem();

        RaytracedGeometryDrawingSubsystem(RaytracedGeometryDrawingSubsystem& other)            = delete;
        RaytracedGeometryDrawingSubsystem& operator=(RaytracedGeometryDrawingSubsystem& other) = delete;

        RaytracedGeometryDrawingSubsystem(RaytracedGeometryDrawingSubsystem&& other)            = delete;
        RaytracedGeometryDrawingSubsystem& operator=(RaytracedGeometryDrawingSubsystem&& other) = delete;

        void Update(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::VkStagingBufferResource& stagingBuffer);

        void Render(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::RenderTargetResource& renderTarget);

    private:
        inline static const Resources::Resource::ID SBT_BUFFER_ID = std::hash<std::string_view>{}("RaytracedGeometryDrawingSubsystem/SBTBufferId");
        inline static const Resources::Resource::ID TLAS_INSTANCES_BUFFER_ID =
            std::hash<std::string_view>{}("RaytracedGeometryDrawingSubsystem/TLASInstancesBufferId");
        inline static const Resources::Resource::ID TLAS_ACCEL_STRUCT_BUFFER_ID =
            std::hash<std::string_view>{}("RaytracedGeometryDrawingSubsystem/TLASAccelStructBufferId");

        Resources::ContextResources& _contextResources;

        VkDescriptorPool             _descriptorPool      = VK_NULL_HANDLE;
        VkDescriptorSetLayout        _descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> _descriptorSets      = {};
        VkPipelineLayout             _pipelineLayout      = VK_NULL_HANDLE;
        VkPipeline                   _pipeline            = VK_NULL_HANDLE;

        VkStridedDeviceAddressRegionKHR _raygenShaderRegion{};
        VkStridedDeviceAddressRegionKHR _missShaderRegion{};
        VkStridedDeviceAddressRegionKHR _hitShaderRegion{};
        VkStridedDeviceAddressRegionKHR _callableShaderRegion{};

        std::unordered_map<Resources::MeshResource::ID, std::vector<entt::entity>> _blasToInstanceData;

        // When recreating TLAS.
        std::optional<Resources::VkBufferResource<VkAccelerationStructureInstanceKHR>> _tlasInstancesBufferToDelete = std::nullopt;
        std::optional<Resources::VkAccelerationStructureResource>                      _tlasAccelStructToDelete     = std::nullopt;
    };
}; // namespace Prism::Systems::Subsystems::MeshDrawingSystem