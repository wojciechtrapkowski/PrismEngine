#pragma once

#include "resources/mesh_resource.hpp"

#include <entt/entt.hpp>

#include "resources/resource.hpp"

#include <optional>
#include <utility>

namespace Prism::Resources {
    struct Scene : ResourceImpl<Scene> {
        Scene() = default;
        ~Scene() = default;

        Scene &operator=(Scene &&other) = default;
        Scene(Scene &&other) = default;

        // For now, will be implemented in the future. Scene should be copyable.
        Scene(Scene &other) = delete;
        Scene &operator=(Scene &other) = delete;

        entt::registry &GetRegistry() { return m_registry; }

        std::optional<std::reference_wrapper<const MeshResource>>
        GetMesh(Resources::MeshResource::ID resourceId);

        void AddNewMesh(Resources::MeshResource::ID id,
                        std::unique_ptr<Resources::MeshResource> meshResource);

        void RemoveMesh(Resources::MeshResource::ID meshId);

      private:
        entt::registry m_registry;

        std::unordered_map<Resources::MeshResource::ID,
                           std::unique_ptr<Resources::MeshResource>>
            m_meshes;
    };
}; // namespace Prism::Resources