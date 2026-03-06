#include "systems/mesh_drawing_system.hpp"

#include <filesystem>
#include <iostream>
#include <vector>

#include "systems/subsystems/rasterized_geometry_drawing_subsystem.hpp"
#include "systems/subsystems/raytraced_geometry_drawing_subsystem.hpp"

#include "vulkan/vulkan.h"

namespace Prism::Systems
{

    MeshDrawingSystem::MeshDrawingSystem(Resources::ContextResources& contextResources) : _contextResources(contextResources)
    {
        _rasterizedGeometryDrawingSubsystem = std::make_unique<Subsystems::MeshDrawingSystem::RasterizedGeometryDrawingSubsystem>(contextResources);
        _raytracedGeometryDrawingSubsystem  = std::make_unique<Subsystems::MeshDrawingSystem::RaytracedGeometryDrawingSubsystem>(contextResources);
    };

    MeshDrawingSystem::~MeshDrawingSystem() {}

    void MeshDrawingSystem::Initialize(){

    };

    void MeshDrawingSystem::Update(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::VkStagingBufferResource& stagingBuffer)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        _rasterizedGeometryDrawingSubsystem->Update(deltaTime, commandBuffer, scene);
        _raytracedGeometryDrawingSubsystem->Update(deltaTime, commandBuffer, scene, stagingBuffer);

        vkEndCommandBuffer(commandBuffer);
    };

    void MeshDrawingSystem::Render(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::RenderTargetResource& renderTarget)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        _rasterizedGeometryDrawingSubsystem->Render(deltaTime, commandBuffer, scene, renderTarget);
        _raytracedGeometryDrawingSubsystem->Render(deltaTime, commandBuffer, scene, renderTarget);

        vkEndCommandBuffer(commandBuffer);
    }
} // namespace Prism::Systems