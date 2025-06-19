#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::Systems {
    class PresentSystem {
      public:
        PresentSystem(Resources::ContextResources &contextResources);
        ~PresentSystem() = default;

        PresentSystem(PresentSystem &other) = delete;
        PresentSystem &operator=(PresentSystem &other) = delete;

        PresentSystem(PresentSystem &&other) = delete;
        PresentSystem &operator=(PresentSystem &&other) = delete;

        void Initialize();

        void Update(float deltaTime, Resources::Scene &scene);

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        Resources::ContextResources &m_contextResources;
    };
}; // namespace Prism::Systems