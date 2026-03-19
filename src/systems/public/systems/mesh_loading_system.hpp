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

        void Update(float deltaTime, Resources::Scene& scene, Resources::VkStagingBufferResource& stagingBuffer);

    private:
        void onMeshFileOpen(const Events::MeshFileOpenEvent& event);

        Resources::ContextResources& _contextResources;

        entt::scoped_connection _onMeshFileOpenConnection;

        std::optional<std::string> _meshFilePathToLoad;
    };
}; // namespace Prism::Systems