#pragma once

#include "systems/camera_creation_system.hpp"
#include "systems/common_uniform_update_system.hpp"
#include "systems/fps_motion_control_system.hpp"

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::Managers {
    class SceneUpdateSystemsManager {
      public:
        SceneUpdateSystemsManager(
            Resources::ContextResources &contextResources);
        ~SceneUpdateSystemsManager() = default;

        SceneUpdateSystemsManager(const SceneUpdateSystemsManager &) = delete;
        SceneUpdateSystemsManager &
        operator=(const SceneUpdateSystemsManager &) = delete;

        SceneUpdateSystemsManager(SceneUpdateSystemsManager &&) = delete;
        SceneUpdateSystemsManager &
        operator=(SceneUpdateSystemsManager &&) = delete;

        void Initialize();
        void Update(float deltaTime, Resources::Scene &scene);

      private:
        Systems::CameraCreationSystem cameraCreationSystem;
        Systems::FpsMotionControlSystem fpsMotionControlSystem;
        Systems::CommonUniformUpdateSystem commonUniformUpdateSystem;
    };
} // namespace Prism::Managers