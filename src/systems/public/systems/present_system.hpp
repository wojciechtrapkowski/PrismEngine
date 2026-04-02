#pragma once

#include <GLFW/glfw3.h>

#include "resources/context_resources.hpp"
#include "resources/render_target_resource.hpp"
#include "resources/scene.hpp"
#include "resources/vulkan/vk_staging_buffer_resource.hpp"

namespace Prism::Systems
{
    class PresentSystem
    {
    public:
        PresentSystem(Resources::ContextResources& contextResources);
        ~PresentSystem() = default;

        PresentSystem(PresentSystem& other)            = delete;
        PresentSystem& operator=(PresentSystem& other) = delete;

        PresentSystem(PresentSystem&& other)            = delete;
        PresentSystem& operator=(PresentSystem&& other) = delete;

        void Update(float deltaTime, VkCommandBuffer commandBuffer, Resources::VkStagingBufferResource& stagingBuffer, Resources::Scene& scene);

        void Render(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::RenderTargetResource& renderTarget);

    private:
        Resources::ContextResources& m_contextResources;
    };
}; // namespace Prism::Systems