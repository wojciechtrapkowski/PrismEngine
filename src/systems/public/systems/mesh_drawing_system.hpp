#pragma once

#include "loaders/shader_loader.hpp"

#include "resources/scene.hpp"
#include "resources/shader_resource.hpp"

namespace Prism::Systems {
    class MeshDrawingSystem {
      public:
        MeshDrawingSystem() = default;
        ~MeshDrawingSystem() = default;

        MeshDrawingSystem(MeshDrawingSystem &other) = delete;
        MeshDrawingSystem &operator=(MeshDrawingSystem &other) = delete;

        MeshDrawingSystem(MeshDrawingSystem &&other) = delete;
        MeshDrawingSystem &operator=(MeshDrawingSystem &&other) = delete;

        void Initialize();

        void Update();

        void Render(Resources::Scene &scene);

      private:
        std::unique_ptr<Resources::ShaderResource> m_shaderResource;
    };
}; // namespace Prism::Systems