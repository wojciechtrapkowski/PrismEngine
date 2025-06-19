#pragma once

#include "systems/mesh_drawing_system.hpp"
#include "systems/present_system.hpp"
#include "systems/screen_clearing_system.hpp"
#include "systems/ui_drawing_system.hpp"

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::Managers {
    class SceneDrawSystemsManager {
      public:
        SceneDrawSystemsManager(Resources::ContextResources &contextResources);

        ~SceneDrawSystemsManager() = default;

        SceneDrawSystemsManager(const SceneDrawSystemsManager &) = delete;
        SceneDrawSystemsManager &
        operator=(const SceneDrawSystemsManager &) = delete;

        SceneDrawSystemsManager(SceneDrawSystemsManager &&) = delete;
        SceneDrawSystemsManager &operator=(SceneDrawSystemsManager &&) = delete;

        void Initialize();

        void Update(float deltaTime, Resources::Scene &scene);

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        Systems::ScreenClearingSystem screenClearingSystem;
        Systems::MeshDrawingSystem meshDrawingSystem;
        Systems::UIDrawingSystem uiDrawingSystem;
        Systems::PresentSystem presentSystem;
    };

} // namespace Prism::Managers