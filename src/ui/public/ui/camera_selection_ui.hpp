#pragma once

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::UI {
    class CameraSelectionUI {
      public:
        CameraSelectionUI(Resources::ContextResources &contextResources);
        ~CameraSelectionUI() = default;

        CameraSelectionUI(const CameraSelectionUI &) = delete;
        CameraSelectionUI &operator=(const CameraSelectionUI &) = delete;

        CameraSelectionUI(CameraSelectionUI &&) = delete;
        CameraSelectionUI &operator=(CameraSelectionUI &&) = delete;

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        Resources::ContextResources &m_contextResources;
    };
} // namespace Prism::UI