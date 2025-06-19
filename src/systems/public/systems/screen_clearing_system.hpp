#pragma once

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::Systems {
    class ScreenClearingSystem {
      public:
        ScreenClearingSystem(Resources::ContextResources &contextResources);
        ~ScreenClearingSystem() = default;

        ScreenClearingSystem(ScreenClearingSystem &other) = delete;
        ScreenClearingSystem &operator=(ScreenClearingSystem &other) = delete;

        ScreenClearingSystem(ScreenClearingSystem &&other) = delete;
        ScreenClearingSystem &operator=(ScreenClearingSystem &&other) = delete;

        void Initialize();

        void Update(float deltaTime, Resources::Scene &scene);

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        Resources::ContextResources &m_contextResources;
    };
}; // namespace Prism::Systems