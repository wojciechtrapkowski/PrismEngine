#include "resources/scene.hpp"

#include "components/mesh.hpp"

#include <functional>

namespace Prism::Resources {

    std::optional<std::reference_wrapper<const MeshResource>>
    Scene::GetMesh(Resources::MeshResource::ID resourceId) {
        auto it = m_meshes.find(resourceId);
        if (it == m_meshes.end()) {
            return std::nullopt;
        }

        return std::cref(*(it->second));
    }

    void
    Scene::AddNewMesh(Resources::MeshResource::ID id,
                      std::unique_ptr<Resources::MeshResource> meshResource) {
        auto newMeshEntity = m_registry.create();
        m_registry.emplace<Components::Mesh>(newMeshEntity, id);

        m_meshes.insert({id, std::move(meshResource)});
    }

    void Scene::RemoveMesh(Resources::MeshResource::ID meshId) {
        // Remove entities associated with this component
        auto meshView = m_registry.view<Components::Mesh>();
        for (auto entity : meshView) {
            if (meshView.get<Components::Mesh>(entity).resourceId == meshId) {
                m_registry.remove<Components::Mesh>(entity);
            }
        }
        m_meshes.erase(meshId);
    }

} // namespace Prism::Resources