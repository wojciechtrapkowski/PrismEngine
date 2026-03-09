#pragma once

#include "resources/context_resources.hpp"
#include "resources/render_target_resource.hpp"
#include "resources/scene.hpp"
#include "resources/vulkan/vk_staging_buffer_resource.hpp"

namespace Prism::Systems
{
    namespace Subsystems::MeshDrawingSystem
    {
        class RasterizedGeometryDrawingSubsystem;
        class RaytracedGeometryDrawingSubsystem;
    } // namespace Subsystems::MeshDrawingSystem
    class MeshDrawingSystem
    {
    public:
        MeshDrawingSystem(Resources::ContextResources& contextResources);
        ~MeshDrawingSystem();

        MeshDrawingSystem(MeshDrawingSystem& other)            = delete;
        MeshDrawingSystem& operator=(MeshDrawingSystem& other) = delete;

        MeshDrawingSystem(MeshDrawingSystem&& other)            = delete;
        MeshDrawingSystem& operator=(MeshDrawingSystem&& other) = delete;

        void Update(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::VkStagingBufferResource& stagingBuffer);

        void Render(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::RenderTargetResource& renderTarget);

    private:
        Resources::ContextResources& _contextResources;

        std::unique_ptr<Subsystems::MeshDrawingSystem::RasterizedGeometryDrawingSubsystem> _rasterizedGeometryDrawingSubsystem;
        std::unique_ptr<Subsystems::MeshDrawingSystem::RaytracedGeometryDrawingSubsystem>  _raytracedGeometryDrawingSubsystem;
    };
}; // namespace Prism::Systems