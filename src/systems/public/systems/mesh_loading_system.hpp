#pragma once

#include <GLFW/glfw3.h>

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"
#include "resources/vulkan/vk_staging_buffer_resource.hpp"

#include "events/file_events.hpp"

namespace Prism::Systems
{
    // Will manage Vertex Index and Texture buffer lifetime. If the new request for a mesh came - load it using mesh loader.
    // Check if we need to create / recreate buffer - if not just use staging buffer to copy. If will be needed we can use another
    // thread to memcpy everything. Create MeshResources - which will contain BuffersAllocation for each. Then UI will just take all of these
    // MeshResources and add option to select one for entity. MeshDrawingSystem & RT System will use these vertex and index buffers. Texture as well
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