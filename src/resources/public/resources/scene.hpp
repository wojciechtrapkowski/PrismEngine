#pragma once

#include "resources/mesh_resource.hpp"

namespace Prism::Resources {
    struct Scene {
        Scene() = default;
        ~Scene() = default;

        Scene &operator=(Scene &&other) = default;
        Scene(Scene &&other) = default;

        // For now, will be implemented in the future. Scene should be copyable.
        Scene(Scene &other) = delete;
        Scene &operator=(Scene &other) = delete;

        const std::vector<std::unique_ptr<MeshResource>> &GetAllMeshes() const {
            return m_meshes;
        }

        void AddNewMesh(std::unique_ptr<Resources::MeshResource> meshResource) {
            m_meshes.push_back(std::move(meshResource));
        };

        // We will need as well RemoveMesh(Mesh::ID) or something like that in
        // the future.

      private:
        // Will be converted to a map, when we starting using registsry.
        std::vector<std::unique_ptr<MeshResource>> m_meshes;
    };
}; // namespace Prism::Resources