#pragma once

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::UI {
    class CameraSettingsUI {
      public:
        CameraSettingsUI(Resources::ContextResources &contextResources);
        ~CameraSettingsUI() = default;

        CameraSettingsUI(const CameraSettingsUI &) = delete;
        CameraSettingsUI &operator=(const CameraSettingsUI &) = delete;

        CameraSettingsUI(CameraSettingsUI &&) = delete;
        CameraSettingsUI &operator=(CameraSettingsUI &&) = delete;

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        Resources::ContextResources &m_contextResources;
    };
} // namespace Prism::UI