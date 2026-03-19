#include "systems/mesh_loading_system.hpp"

#include "loaders/mesh_loader.hpp"

#include <iostream>

namespace Prism::Systems
{
    namespace
    {} // namespace

    MeshLoadingSystem::MeshLoadingSystem(Resources::ContextResources& contextResources) : _contextResources(contextResources)
    {
        auto& dispatcher = _contextResources.GetDispatcher();

        _onMeshFileOpenConnection = dispatcher.sink<Events::MeshFileOpenEvent>().connect<&MeshLoadingSystem::onMeshFileOpen>(this);
    };

    void MeshLoadingSystem::Update(float deltaTime, Resources::Scene& scene, Resources::VkStagingBufferResource& stagingBuffer)
    {
        auto& registry = scene.GetRegistry();

        if (_meshFilePathToLoad) {
            Loaders::MeshLoader meshLoader{};

            auto meshResource = meshLoader(_contextResources.GetVulkanResource(), stagingBuffer, *_meshFilePathToLoad);
            if (!meshResource) {
#ifdef DEBUG
                std::cerr << "Couldn't load mesh from file: " << *_meshFilePathToLoad << std::endl;
#endif
                return;
            }

            auto& meshStorage = scene.GetMeshStorage();
            auto  meshId      = std::hash<std::string>{}("MeshResources/" + *_meshFilePathToLoad);

            if (!meshStorage.Contains(meshId)) {
                meshStorage.Insert<Resources::MeshResource>(meshId, std::move(*meshResource));
            }

            _meshFilePathToLoad = std::nullopt;
        }
    }

    void MeshLoadingSystem::onMeshFileOpen(const Events::MeshFileOpenEvent& event)
    {
        _meshFilePathToLoad = event.filePath;
    }

} // namespace Prism::Systems
