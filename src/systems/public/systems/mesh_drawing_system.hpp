#pragma once

#include "loaders/shader_loader.hpp"

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"
#include "resources/shader_resource.hpp"

namespace Prism::Systems {
    class MeshDrawingSystem {
      public:
        MeshDrawingSystem(Resources::ContextResources &contextResources);
        ~MeshDrawingSystem() = default;

        MeshDrawingSystem(MeshDrawingSystem &other) = delete;
        MeshDrawingSystem &operator=(MeshDrawingSystem &other) = delete;

        MeshDrawingSystem(MeshDrawingSystem &&other) = delete;
        MeshDrawingSystem &operator=(MeshDrawingSystem &&other) = delete;

        void Initialize();

        void Update(float deltaTime, Resources::Scene &scene);

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        Resources::ContextResources &m_contextResources;

        std::unique_ptr<Resources::ShaderResource> m_shaderResource;
    };
}; // namespace Prism::Systems