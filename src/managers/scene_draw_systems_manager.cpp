#include "managers/scene_draw_systems_manager.hpp"

namespace Prism::Managers {
    SceneDrawSystemsManager::SceneDrawSystemsManager(
        Resources::ContextResources &contextResources)
        : screenClearingSystem{contextResources},
          meshDrawingSystem{contextResources},
          uiDrawingSystem{contextResources}, presentSystem{contextResources},
          gizmoDrawingSystem{contextResources} {}

    void SceneDrawSystemsManager::Initialize() {
        screenClearingSystem.Initialize();
        meshDrawingSystem.Initialize();
        gizmoDrawingSystem.Initialize();
        uiDrawingSystem.Initialize();
        presentSystem.Initialize();
    }

    void SceneDrawSystemsManager::Update(float deltaTime,
                                         Resources::Scene &scene) {
        screenClearingSystem.Update(deltaTime, scene);
        meshDrawingSystem.Update(deltaTime, scene);
        uiDrawingSystem.Update(deltaTime, scene);
        gizmoDrawingSystem.Update(deltaTime, scene);
        presentSystem.Update(deltaTime, scene);
    }

    void SceneDrawSystemsManager::Render(float deltaTime,
                                         Resources::Scene &scene) {
        screenClearingSystem.Render(deltaTime, scene);
        meshDrawingSystem.Render(deltaTime, scene);
        gizmoDrawingSystem.Render(deltaTime, scene);
        uiDrawingSystem.Render(deltaTime, scene);
        presentSystem.Render(deltaTime, scene);
    }
} // namespace Prism::Managers