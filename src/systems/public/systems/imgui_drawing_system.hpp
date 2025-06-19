#pragma once

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::Systems {
    class ImGuiDrawingSystem {
      public:
        ImGuiDrawingSystem(Resources::ContextResources &contextResources);
        ~ImGuiDrawingSystem() = default;

        ImGuiDrawingSystem(ImGuiDrawingSystem &other) = delete;
        ImGuiDrawingSystem &operator=(ImGuiDrawingSystem &other) = delete;

        ImGuiDrawingSystem(ImGuiDrawingSystem &&other) = delete;
        ImGuiDrawingSystem &operator=(ImGuiDrawingSystem &&other) = delete;

        void Initialize();

        void Update(float deltaTime, Resources::Scene &scene);

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        Resources::ContextResources &m_contextResources;
    };
}; // namespace Prism::Systems