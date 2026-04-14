#pragma once

#include <GLFW/glfw3.h>

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"
#include "resources/vulkan/vk_staging_buffer_resource.hpp"

#include "events/file_events.hpp"

namespace Prism::Systems
{
    struct MeshLoadingSystem
    {
        MeshLoadingSystem(Resources::ContextResources& contextResources);
        ~MeshLoadingSystem() = default;

        MeshLoadingSystem(MeshLoadingSystem& other)            = delete;
        MeshLoadingSystem& operator=(MeshLoadingSystem& other) = delete;

        MeshLoadingSystem(MeshLoadingSystem&& other)            = delete;
        MeshLoadingSystem& operator=(MeshLoadingSystem&& other) = delete;

        void Update(float deltaTime, VkCommandBuffer commandBuffer, Resources::VkStagingBufferResource& stagingBuffer, Resources::Scene& scene);

    private:
        void onMeshFileOpen(const Events::MeshFileOpenEvent& event);

        Resources::ContextResources& _contextResources;

        entt::scoped_connection _onMeshFileOpenConnection;

        std::optional<std::string> _meshFilePathToLoad;

        std::vector<std::string> _meshesLoaded;
    };
}; // namespace Prism::Systems