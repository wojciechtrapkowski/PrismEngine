#include "systems/mesh_drawing_system.hpp"

#include <filesystem>
#include <iostream>
#include <vector>

#include "systems/subsystems/rasterized_geometry_drawing_subsystem.hpp"
#include "systems/subsystems/raytraced_geometry_drawing_subsystem.hpp"

#include "components/systems_settings.hpp"

#include "vulkan/vulkan.h"

namespace Prism::Systems
{

    MeshDrawingSystem::MeshDrawingSystem(Resources::ContextResources& contextResources) : _contextResources(contextResources)
    {
        _rasterizedGeometryDrawingSubsystem = std::make_unique<Subsystems::MeshDrawingSystem::RasterizedGeometryDrawingSubsystem>(contextResources);
        _raytracedGeometryDrawingSubsystem  = std::make_unique<Subsystems::MeshDrawingSystem::RaytracedGeometryDrawingSubsystem>(contextResources);
    };

    MeshDrawingSystem::~MeshDrawingSystem() {}

    void MeshDrawingSystem::Update(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::VkStagingBufferResource& stagingBuffer)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        auto& registry            = scene.GetRegistry();
        auto  systemsSettingsView = registry.view<Components::MeshDrawingSystemSettings>();

        if (systemsSettingsView.empty()) {
            auto systemsSettingsEntity = registry.create();
            registry.emplace<Components::MeshDrawingSystemSettings>(systemsSettingsEntity);
            systemsSettingsView = registry.view<Components::MeshDrawingSystemSettings>();
        }

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

        auto& registry            = scene.GetRegistry();
        auto  systemsSettingsView = registry.view<Components::MeshDrawingSystemSettings>();
        if (systemsSettingsView.empty()) {
            throw std::runtime_error("MeshDrawingSystemSettings component is missing!");
        }

        auto& settings = systemsSettingsView.get<Components::MeshDrawingSystemSettings>(systemsSettingsView.front());

        if (settings.drawingMode == Components::MeshDrawingSystemSettings::MeshDrawingMode::RASTERIZATION) {
            _rasterizedGeometryDrawingSubsystem->Render(deltaTime, commandBuffer, scene, renderTarget);
        } else {
            _raytracedGeometryDrawingSubsystem->Render(deltaTime, commandBuffer, scene, renderTarget);
        }

        vkEndCommandBuffer(commandBuffer);
    }
} // namespace Prism::Systems