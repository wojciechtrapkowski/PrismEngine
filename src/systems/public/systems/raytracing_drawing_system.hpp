#pragma once

#include "resources/vulkan_resource.hpp"
#include "resources/scene.hpp"
#include "resources/context_resources.hpp"
#include "resources/render_target_resource.hpp"

namespace Prism::Systems
{
    struct RaytracingDrawingSystem
    {
    public:
        RaytracingDrawingSystem(Resources::ContextResources& contextResources);
        ~RaytracingDrawingSystem();

        RaytracingDrawingSystem(RaytracingDrawingSystem& other)            = delete;
        RaytracingDrawingSystem& operator=(RaytracingDrawingSystem& other) = delete;

        RaytracingDrawingSystem(RaytracingDrawingSystem&& other)            = delete;
        RaytracingDrawingSystem& operator=(RaytracingDrawingSystem&& other) = delete;

        void Initialize();

        void Update(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene);

        void Render(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::RenderTargetResource& renderTarget);

    private:
        Resources::ContextResources& _contextResources;

        VkDescriptorPool             _descriptorPool      = VK_NULL_HANDLE;
        VkDescriptorSetLayout        _descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> _descriptorSets      = {};
        VkPipelineLayout             _pipelineLayout      = VK_NULL_HANDLE;
        VkPipeline                   _pipeline            = VK_NULL_HANDLE;
    };
}; // namespace Prism::Systems