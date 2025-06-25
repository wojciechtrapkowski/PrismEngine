#include "systems/camera_creation_system.hpp"

#include "components/camera.hpp"
#include "components/fps_camera_control.hpp"
#include "components/tags.hpp"
#include "components/transform.hpp"

#include "events/move_events.hpp"

namespace Prism::Systems {
    namespace {
        void createFpsCamera(entt::registry &registry,
                             entt::entity cameraEntity) {
            registry.emplace<Components::FpsCameraControl>(cameraEntity);
        }
    } // namespace

    CameraCreationSystem::CameraCreationSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {};

    void CameraCreationSystem::Initialize() {

    };

    void CameraCreationSystem::Update(float deltaTime,
                                      Resources::Scene &scene) {
        // In the future there will be more camera types.

        auto &registry = scene.GetRegistry();

        auto activeCameraView = registry.view<Components::Tags::ActiveCamera>();
        if (!activeCameraView.empty()) {
            return;
        }

        auto cameraEntity = registry.create();
        registry.emplace<Components::Tags::ActiveCamera>(cameraEntity);
        registry.emplace<Components::Camera>(cameraEntity);
        registry.emplace<Components::Transform>(cameraEntity);

        createFpsCamera(registry, cameraEntity);
    };
} // namespace Prism::Systems