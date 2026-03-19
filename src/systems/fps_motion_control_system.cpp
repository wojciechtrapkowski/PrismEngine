#include "systems/fps_motion_control_system.hpp"

#include "components/camera.hpp"
#include "components/fps_camera_control.hpp"
#include "components/tags.hpp"
#include "components/transform.hpp"

#include "events/move_events.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>

namespace Prism::Systems
{
    namespace
    {
        constexpr float     PITCH_LIMIT          = 89.0f;
        constexpr glm::vec3 WORLD_FORWARD_VECTOR = {0.0f, 0.0f, -1.0f};
        constexpr glm::vec3 WORLD_RIGHT_VECTOR   = {1.0f, 0.0f, 0.0f};
        constexpr glm::vec3 WORLD_UP_VECTOR      = {0.0f, 1.0f, 0.0f};
        constexpr float     MIN_FOCUS_DISTANCE   = 1.0f;

    } // namespace

    FpsMotionControlSystem::FpsMotionControlSystem(Resources::ContextResources& contextResources) : m_contextResources(contextResources)
    {
        auto& dispatcher = m_contextResources.GetDispatcher();

        m_onKeyPressedConnection = dispatcher.sink<Events::KeyPressEvent>().connect<&FpsMotionControlSystem::onKeyPressed>(this);

        m_onMousePressedConnection = dispatcher.sink<Events::MouseButtonPressEvent>().connect<&FpsMotionControlSystem::onMousePressed>(this);

        m_onMouseMovementConnection = dispatcher.sink<Events::MouseMoveEvent>().connect<&FpsMotionControlSystem::onMouseMoved>(this);

        m_onMouseScrollConnection = dispatcher.sink<Events::MouseScrollEvent>().connect<&FpsMotionControlSystem::onMouseScrolled>(this);
    };

    void FpsMotionControlSystem::Update(float deltaTime, Resources::Scene& scene)
    {
        auto& registry = scene.GetRegistry();

        auto activeCameraView = registry.view<Components::Tags::ActiveCamera>();
        if (activeCameraView.empty()) {
            return;
        }

        auto playerView = registry.view<Components::Tags::ActivePlayer>();
        if (playerView.empty()) {
            return;
        }

        if (!registry.all_of<Components::Camera, Components::CameraControl, Components::Transform>(activeCameraView.front())) {
            return;
        }

        if (!registry.all_of<Components::Transform>(playerView.front())) {
            return;
        }

        auto cameraEntity = activeCameraView.front();

        auto& cameraComponent        = registry.get<Components::Camera>(cameraEntity);
        auto& cameraControlComponent = registry.get<Components::CameraControl>(cameraEntity);
        auto& cameraTransform        = registry.get<Components::Transform>(cameraEntity);

        auto cameraType = [&]() {
            if (cameraControlComponent.currentDistance <= MIN_FOCUS_DISTANCE) {
                return CameraType::FirstPerson;
            } else {
                return CameraType::ThirdPerson;
            }
        }();

        auto playerEntity = playerView.front();

        auto& playerTransform = registry.get<Components::Transform>(playerEntity);

        auto playerPosition = glm::vec3(playerTransform.transform[3]);
        auto playerRotation = glm::mat4(glm::mat3(playerTransform.transform));

        auto cameraPosition = glm::vec3(cameraTransform.transform[3]);
        auto cameraRotation = glm::mat4(glm::mat3(cameraTransform.transform));

        float playerPitch, playerYaw, playerRoll;
        float cameraPitch, cameraYaw, cameraRoll;

        glm::extractEulerAngleYXZ(playerRotation, playerYaw, playerPitch, playerRoll);
        glm::extractEulerAngleYXZ(cameraRotation, cameraYaw, cameraPitch, cameraRoll);

        playerPitch = glm::degrees(playerPitch);
        playerYaw   = glm::degrees(playerYaw);
        playerRoll  = glm::degrees(playerRoll);

        cameraPitch = glm::degrees(cameraPitch);
        cameraYaw   = glm::degrees(cameraYaw);
        cameraRoll  = glm::degrees(cameraRoll);

        if (m_mouseButtonToStateMap[Events::MoveEvents::MouseButton::Left] == Events::MoveEvents::InputAction::Pressed) {
            float deltaX = m_mousePositionDelta.first * cameraControlComponent.mouseSensitivity;
            float deltaY = m_mousePositionDelta.second * cameraControlComponent.mouseSensitivity;

            playerYaw -= deltaX;
            playerPitch += deltaY;

            playerPitch = glm::clamp(playerPitch, -PITCH_LIMIT, PITCH_LIMIT);
        }

        if (m_mouseButtonToStateMap[Events::MoveEvents::MouseButton::Right] == Events::MoveEvents::InputAction::Pressed) {
            float deltaX = m_mousePositionDelta.first * cameraControlComponent.mouseSensitivity;
            float deltaY = m_mousePositionDelta.second * cameraControlComponent.mouseSensitivity;

            cameraYaw -= deltaX;
            cameraPitch += deltaY;

            cameraPitch = glm::clamp(cameraPitch, -PITCH_LIMIT, PITCH_LIMIT);
        }

        playerYaw   = glm::radians(playerYaw);
        playerPitch = glm::radians(playerPitch);

        cameraYaw   = glm::radians(cameraYaw);
        cameraPitch = glm::radians(cameraPitch);

        if (cameraType == CameraType::ThirdPerson) {
            playerPitch = 0.f;
        }

        playerRotation = glm::eulerAngleYXZ(playerYaw, playerPitch, 0.0f);
        cameraRotation = glm::eulerAngleYXZ(cameraYaw, cameraPitch, 0.0f);

        auto playerRight   = glm::vec3(playerRotation[0]);
        auto playerUp      = glm::vec3(playerRotation[1]);
        auto playerForward = -glm::vec3(playerRotation[2]);

        float flipSign = cameraType == CameraType::FirstPerson ? 1.f : -1.f;
        if (m_keyToStateMap[Events::MoveEvents::Keys::W] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition += flipSign * playerForward * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::S] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition -= flipSign * playerForward * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::A] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition -= flipSign * playerRight * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::D] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition += flipSign * playerRight * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::SPACE] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition += flipSign * WORLD_UP_VECTOR * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (m_keyToStateMap[Events::MoveEvents::Keys::SHIFT] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition -= flipSign * WORLD_UP_VECTOR * cameraControlComponent.moveSpeed * deltaTime;
        }

        cameraControlComponent.currentDistance -= m_scrollOffset.second * cameraControlComponent.moveSpeed * deltaTime;
        cameraControlComponent.currentDistance = glm::max(cameraControlComponent.currentDistance, MIN_FOCUS_DISTANCE);

        glm::mat4 rotationMat     = playerRotation;
        glm::mat4 translationMat  = glm::translate(glm::mat4(1.0f), playerPosition);
        playerTransform.transform = translationMat * rotationMat;

        // Camera
        float cameraDistance = cameraControlComponent.currentDistance;
        float cameraHeight   = 2.0f;

        // Spherical coordinates: distance is constant, yaw/pitch just change the orbit point
        float x = cameraDistance * cos(cameraPitch) * sin(cameraYaw);
        float y = cameraDistance * sin(cameraPitch);
        float z = cameraDistance * cos(cameraPitch) * cos(cameraYaw);

        cameraPosition = playerPosition + glm::vec3(x, y, z);

        cameraTransform.transform = glm::translate(glm::mat4(1.0f), cameraPosition) * cameraRotation;

        glm::vec3 lookTarget = playerPosition + WORLD_UP_VECTOR * 0.5f;

        glm::mat4 view = [&]() {
            if (cameraDistance == MIN_FOCUS_DISTANCE) {
                return glm::lookAt(playerPosition, playerPosition + playerForward, WORLD_UP_VECTOR);
            } else {
                return glm::lookAt(cameraPosition, lookTarget, WORLD_UP_VECTOR);
            }
        }();

        auto [width, height] = m_contextResources.GetVulkanResource().GetSwapchainExtent();

        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

        glm::mat4 projection =
            glm::perspective(glm::radians(cameraControlComponent.fov), aspectRatio, cameraControlComponent.nearPlane, cameraControlComponent.farPlane);

        cameraComponent.view       = std::move(view);
        cameraComponent.projection = std::move(projection);

        m_mousePositionDelta = {0.f, 0.f};
        m_scrollOffset       = {0.f, 0.f};
        m_keyToStateMap.clear();
        m_mouseButtonToStateMap.clear();
    }

    void FpsMotionControlSystem::onKeyPressed(const Events::KeyPressEvent& event)
    {
        m_keyToStateMap[event.key] = event.action;
    }

    void FpsMotionControlSystem::onMousePressed(const Events::MouseButtonPressEvent& event)
    {
        m_mouseButtonToStateMap[event.button] = event.action;
    }

    void FpsMotionControlSystem::onMouseMoved(const Events::MouseMoveEvent& event)
    {
        m_mousePositionDelta = {event.position.first - m_mousePosition.first, m_mousePosition.second - event.position.second};

        // Skip first update.
        if (m_mousePosition == std::pair<double, double>{0, 0}) {
            m_mousePositionDelta = {0, 0};
        }
        m_mousePosition = {event.position.first, event.position.second};
    }

    void FpsMotionControlSystem::onMouseScrolled(const Events::MouseScrollEvent& event)
    {
        m_scrollOffset = event.scrollOffset;
    }

} // namespace Prism::Systems
