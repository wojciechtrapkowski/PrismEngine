#include "systems/fps_motion_control_system.hpp"

#include "components/fps_camera_control.hpp"
#include "components/fps_motion_control.hpp"
#include "components/tags.hpp"
#include "components/transform.hpp"

#include "events/move_events.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>

namespace Prism::Systems {
    namespace {
        constexpr float PITCH_LIMIT = 89.0f;
        constexpr glm::vec3 WORLD_FORWARD_VECTOR = {0.0f, 0.0f, -1.0f};
        constexpr glm::vec3 WORLD_RIGHT_VECTOR = {1.0f, 0.0f, 0.0f};
        constexpr glm::vec3 WORLD_UP_VECTOR = {0.0f, 1.0f, 0.0f};
    } // namespace

    FpsMotionControlSystem::FpsMotionControlSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {
        auto &dispatcher = m_contextResources.dispatcher;

        m_onKeyPressedConnection =
            dispatcher.sink<Events::KeyPressEvent>()
                .connect<&FpsMotionControlSystem::onKeyPressed>(this);

        m_onMousePressedConnection =
            dispatcher.sink<Events::MouseButtonPressEvent>()
                .connect<&FpsMotionControlSystem::onMousePressed>(this);

        m_onMouseMovementConnection =
            dispatcher.sink<Events::MouseMoveEvent>()
                .connect<&FpsMotionControlSystem::onMouseMoved>(this);
    };

    void FpsMotionControlSystem::Initialize() {

    };

    void FpsMotionControlSystem::Update(float deltaTime,
                                        Resources::Scene &scene) {
        auto &registry = scene.GetRegistry();

        auto activeCameraView = registry.view<Components::Tags::ActiveCamera>();
        if (activeCameraView.empty()) {
            return;
        }

        if (!registry.all_of<Components::FpsMotionControl,
                             Components::FpsCameraControl>(
                activeCameraView.front())) {
            return;
        }

        auto fpsCamera = activeCameraView.front();

        auto &fpsMotionControl =
            registry.get<Components::FpsMotionControl>(fpsCamera);
        auto &cameraControl =
            registry.get<Components::FpsCameraControl>(fpsCamera);

        auto &cameraPosition = fpsMotionControl.cameraPosition;
        auto &cameraForward = fpsMotionControl.cameraForward;
        auto &cameraRight = fpsMotionControl.cameraRight;
        auto &cameraUp = fpsMotionControl.cameraUp;

        if (m_mouseButtonToStateMap[Events::MoveEvents::MouseButton::Left] ==
            Events::MoveEvents::InputAction::Pressed) {
            float deltaX =
                m_mousePositionDelta.first * cameraControl.mouseSensitivity;
            float deltaY =
                m_mousePositionDelta.second * cameraControl.mouseSensitivity;

            fpsMotionControl.yaw += deltaX;
            fpsMotionControl.pitch += deltaY;

            if (fpsMotionControl.pitch > PITCH_LIMIT) {
                fpsMotionControl.pitch = PITCH_LIMIT;
            }

            if (fpsMotionControl.pitch < -PITCH_LIMIT) {
                fpsMotionControl.pitch = -PITCH_LIMIT;
            }
        }

        float yaw = glm::radians(fpsMotionControl.yaw);
        float pitch = glm::radians(fpsMotionControl.pitch);

        cameraForward.x = cos(yaw) * cos(pitch);
        cameraForward.y = sin(pitch);
        cameraForward.z = cos(pitch) * sin(yaw);
        cameraForward = glm::normalize(cameraForward);

        cameraRight =
            glm::normalize(glm::cross(cameraForward, WORLD_UP_VECTOR));

        cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));

        if (m_keyToStateMap[Events::MoveEvents::Keys::W] ==
            Events::MoveEvents::InputAction::Pressed) {
            cameraPosition +=
                cameraForward * cameraControl.moveSpeed * deltaTime;
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::S] ==
            Events::MoveEvents::InputAction::Pressed) {
            cameraPosition -=
                cameraForward * cameraControl.moveSpeed * deltaTime;
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::A] ==
            Events::MoveEvents::InputAction::Pressed) {
            cameraPosition -= cameraRight * cameraControl.moveSpeed * deltaTime;
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::D] ==
            Events::MoveEvents::InputAction::Pressed) {
            cameraPosition += cameraRight * cameraControl.moveSpeed * deltaTime;
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::SPACE] ==
            Events::MoveEvents::InputAction::Pressed) {
            cameraPosition += cameraUp * cameraControl.moveSpeed * deltaTime;
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::SHIFT] ==
            Events::MoveEvents::InputAction::Pressed) {
            cameraPosition -= cameraUp * cameraControl.moveSpeed * deltaTime;
        }

        m_mousePositionDelta = {0.f, 0.f};
        m_keyToStateMap.clear();
        m_mouseButtonToStateMap.clear();
    }

    void
    FpsMotionControlSystem::onKeyPressed(const Events::KeyPressEvent &event) {
        m_keyToStateMap[event.key] = event.action;
    }

    void FpsMotionControlSystem::onMousePressed(
        const Events::MouseButtonPressEvent &event) {
        m_mouseButtonToStateMap[event.button] = event.action;
    }

    void
    FpsMotionControlSystem::onMouseMoved(const Events::MouseMoveEvent &event) {

        m_mousePositionDelta = {event.position.first - m_mousePosition.first,
                                m_mousePosition.second - event.position.second};

        // Skip first update.
        if (m_mousePosition == std::pair<double, double>{0, 0}) {
            m_mousePositionDelta = {0, 0};
        }
        m_mousePosition = {event.position.first, event.position.second};
    }

} // namespace Prism::Systems
