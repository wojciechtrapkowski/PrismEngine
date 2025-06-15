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
        constexpr glm::vec3 FORWARD_VECTOR = {0.0f, 0.0f, -1.0f};
        constexpr glm::vec3 RIGHT_VECTOR = {1.0f, 0.0f, 0.0f};
        constexpr glm::vec3 UP_VECTOR = {0.0f, 1.0f, 0.0f};
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

        if (!registry
                 .all_of<Components::FpsMotionControl, Components::Transform,
                         Components::FpsCameraControl>(
                     activeCameraView.front())) {
            return;
        }

        auto fpsCamera = activeCameraView.front();

        auto &fpsMotionControl =
            registry.get<Components::FpsMotionControl>(fpsCamera);
        auto &cameraControl =
            registry.get<Components::FpsCameraControl>(fpsCamera);
        auto &transform = registry.get<Components::Transform>(fpsCamera);

        auto &pitch = fpsMotionControl.pitch;
        auto &yaw = fpsMotionControl.yaw;

        if (m_mouseButtonToStateMap[Events::MoveEvents::MouseButton::Left] ==
            Events::MoveEvents::InputAction::Pressed) {

            float deltaX =
                m_mousePositionDelta.first * cameraControl.mouseSensitivity;
            float deltaY =
                m_mousePositionDelta.second * cameraControl.mouseSensitivity;

            fpsMotionControl.yaw += deltaX;
            fpsMotionControl.pitch -= deltaY;

            if (fpsMotionControl.pitch > PITCH_LIMIT) {
                fpsMotionControl.pitch = PITCH_LIMIT;
            }

            if (fpsMotionControl.pitch < -PITCH_LIMIT) {
                fpsMotionControl.pitch = -PITCH_LIMIT;
            }
        }


        glm::mat4 rotationMat =
            glm::eulerAngleXYZ(glm::radians(fpsMotionControl.pitch),
                               glm::radians(fpsMotionControl.yaw), 0.0f);

        glm::vec3 forward = -glm::vec3(rotationMat[2]);
        glm::vec3 right = glm::vec3(rotationMat[0]);

        glm::vec3 moveDirection{0.f};

        if (m_keyToStateMap[Events::MoveEvents::Keys::W] ==
            Events::MoveEvents::InputAction::Pressed) {
            moveDirection += glm::vec3(0.0, 0.0, -1.0f);
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::S] ==
            Events::MoveEvents::InputAction::Pressed) {
            moveDirection += glm::vec3(0.0, 0.0, 1.0f);
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::A] ==
            Events::MoveEvents::InputAction::Pressed) {
            moveDirection += glm::vec3(-1.0, 0.0, 0.0f);
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::D] ==
            Events::MoveEvents::InputAction::Pressed) {
            moveDirection += glm::vec3(1.0, 0.0, 0.0f);
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::SPACE] ==
            Events::MoveEvents::InputAction::Pressed) {
            moveDirection += glm::vec3(0.0, 1.0, 0.0f);
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::SHIFT] ==
            Events::MoveEvents::InputAction::Pressed) {
            moveDirection += glm::vec3(0.0, -1.0, 0.0f);
        }

        moveDirection = glm::normalize(moveDirection);

        glm::vec3 moveVector =
            glm::normalize(moveDirection.z * forward + moveDirection.x * right +
                           moveDirection.y * UP_VECTOR);

        glm::vec3 cameraPosition = glm::vec3(transform.transform[3]);
        if (glm::length(moveDirection) > 0.0f) {
            glm::vec3 displacement =
                moveVector * cameraControl.moveSpeed * deltaTime;
            cameraPosition += displacement;
        }

        transform.transform = glm::translate(rotationMat, cameraPosition);

        m_mousePositionDelta = {0.f, 0.f};
        m_keyToStateMap.clear();
        m_mouseButtonToStateMap.clear();
    };

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
        m_mousePositionDelta = {event.delta.first - m_mousePosition.first,
                                event.delta.second - m_mousePosition.second};

        m_mousePosition = {event.delta.first, event.delta.second};
    }

} // namespace Prism::Systems
