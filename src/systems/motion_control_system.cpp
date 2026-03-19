#include "systems/motion_control_system.hpp"

#include "components/camera.hpp"
#include "components/camera_control.hpp"
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
        constexpr float     MIN_FOCUS_DISTANCE   = 5.0f;

    } // namespace

    MotionControlSystem::MotionControlSystem(Resources::ContextResources& contextResources) : _contextResources(contextResources)
    {
        auto& dispatcher = _contextResources.GetDispatcher();

        _onKeyPressedConnection    = dispatcher.sink<Events::KeyPressEvent>().connect<&MotionControlSystem::onKeyPressed>(this);
        _onMousePressedConnection  = dispatcher.sink<Events::MouseButtonPressEvent>().connect<&MotionControlSystem::onMousePressed>(this);
        _onMouseMovementConnection = dispatcher.sink<Events::MouseMoveEvent>().connect<&MotionControlSystem::onMouseMoved>(this);
        _onMouseScrollConnection   = dispatcher.sink<Events::MouseScrollEvent>().connect<&MotionControlSystem::onMouseScrolled>(this);
    };

    void MotionControlSystem::updateFirstPersonCamera(entt::registry& registry, entt::entity cameraEntity, float deltaTime)
    {
        auto& cameraComponent        = registry.get<Components::Camera>(cameraEntity);
        auto& cameraControlComponent = registry.get<Components::CameraControl>(cameraEntity);
        auto& cameraTransform        = registry.get<Components::Transform>(cameraEntity);

        auto cameraPosition = glm::vec3(cameraTransform.transform[3]);
        auto cameraRotation = glm::mat4(glm::mat3(cameraTransform.transform));

        float cameraPitch, cameraYaw, cameraRoll;

        glm::extractEulerAngleYXZ(cameraRotation, cameraYaw, cameraPitch, cameraRoll);
        cameraPitch = glm::degrees(cameraPitch);
        cameraYaw   = glm::degrees(cameraYaw);
        cameraRoll  = glm::degrees(cameraRoll);

        if (_mouseButtonToStateMap[Events::MoveEvents::MouseButton::Left] == Events::MoveEvents::InputAction::Pressed) {
            float deltaX = _mousePositionDelta.first * cameraControlComponent.mouseSensitivity;
            float deltaY = _mousePositionDelta.second * cameraControlComponent.mouseSensitivity;

            cameraYaw -= deltaX;
            cameraPitch += deltaY;

            cameraPitch = glm::clamp(cameraPitch, -PITCH_LIMIT, PITCH_LIMIT);
        }

        cameraYaw   = glm::radians(cameraYaw);
        cameraPitch = glm::radians(cameraPitch);

        cameraRotation = glm::eulerAngleYXZ(cameraYaw, cameraPitch, 0.0f);

        auto cameraRight   = glm::vec3(cameraRotation[0]);
        auto cameraUp      = glm::vec3(cameraRotation[1]);
        auto cameraForward = -glm::vec3(cameraRotation[2]);

        if (_keyToStateMap[Events::MoveEvents::Keys::W] == Events::MoveEvents::InputAction::Pressed) {
            cameraPosition += cameraForward * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (_keyToStateMap[Events::MoveEvents::Keys::S] == Events::MoveEvents::InputAction::Pressed) {
            cameraPosition -= cameraForward * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (_keyToStateMap[Events::MoveEvents::Keys::A] == Events::MoveEvents::InputAction::Pressed) {
            cameraPosition -= cameraRight * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (_keyToStateMap[Events::MoveEvents::Keys::D] == Events::MoveEvents::InputAction::Pressed) {
            cameraPosition += cameraRight * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (_keyToStateMap[Events::MoveEvents::Keys::SPACE] == Events::MoveEvents::InputAction::Pressed) {
            cameraPosition += WORLD_UP_VECTOR * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (_keyToStateMap[Events::MoveEvents::Keys::SHIFT] == Events::MoveEvents::InputAction::Pressed) {
            cameraPosition -= WORLD_UP_VECTOR * cameraControlComponent.moveSpeed * deltaTime;
        }

        cameraTransform.transform = glm::translate(glm::mat4(1.0f), cameraPosition) * cameraRotation;

        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraForward, WORLD_UP_VECTOR);

        cameraComponent.view = std::move(view);
    }

    void MotionControlSystem::updateThirdPersonCamera(entt::registry& registry, entt::entity cameraEntity, float deltaTime)
    {
        auto& cameraComponent        = registry.get<Components::Camera>(cameraEntity);
        auto& cameraControlComponent = registry.get<Components::CameraControl>(cameraEntity);

        auto playerView = registry.view<Components::Tags::ActivePlayer>();
        if (playerView.empty() && cameraControlComponent.cameraType == Components::CameraControl::CameraType::ThirdPerson) {
            // We can't have 3rd person camera without a player to follow.
            std::cerr << "No active player found for third person camera!" << std::endl; // For now, I need to add better error handling.
            return;
        }

        if (playerView.size() > 1) {
            throw std::runtime_error("Multiple entities with ActivePlayer tag found. Third person camera requires exactly one player entity.");
        }

        if (!registry.all_of<Components::Transform>(playerView.front())) {
            return;
        }

        // Handle player.
        auto playerEntity = playerView.front();

        auto& playerTransform = registry.get<Components::Transform>(playerEntity);

        auto playerPosition = glm::vec3(playerTransform.transform[3]);
        auto playerRotation = glm::mat4(glm::mat3(playerTransform.transform));

        float playerPitch, playerYaw, playerRoll;

        glm::extractEulerAngleYXZ(playerRotation, playerYaw, playerPitch, playerRoll);

        playerPitch = glm::degrees(playerPitch);
        playerYaw   = glm::degrees(playerYaw);
        playerRoll  = glm::degrees(playerRoll);

        // Player can move only around y axis.
        if (_mouseButtonToStateMap[Events::MoveEvents::MouseButton::Left] == Events::MoveEvents::InputAction::Pressed) {
            float deltaX = _mousePositionDelta.first * cameraControlComponent.mouseSensitivity;
            float deltaY = _mousePositionDelta.second * cameraControlComponent.mouseSensitivity;

            playerYaw -= deltaX;
        }

        playerYaw = glm::radians(playerYaw);

        playerRotation = glm::eulerAngleYXZ(playerYaw, 0.0f, 0.0f);

        auto playerRight   = glm::vec3(playerRotation[0]);
        auto playerUp      = glm::vec3(playerRotation[1]);
        auto playerForward = -glm::vec3(playerRotation[2]);

        if (_keyToStateMap[Events::MoveEvents::Keys::W] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition -= playerForward * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (_keyToStateMap[Events::MoveEvents::Keys::S] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition += playerForward * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (_keyToStateMap[Events::MoveEvents::Keys::A] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition += playerRight * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (_keyToStateMap[Events::MoveEvents::Keys::D] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition -= playerRight * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (_keyToStateMap[Events::MoveEvents::Keys::SPACE] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition += WORLD_UP_VECTOR * cameraControlComponent.moveSpeed * deltaTime;
        }
        if (_keyToStateMap[Events::MoveEvents::Keys::SHIFT] == Events::MoveEvents::InputAction::Pressed) {
            playerPosition -= WORLD_UP_VECTOR * cameraControlComponent.moveSpeed * deltaTime;
        }

        playerTransform.transform = glm::translate(glm::mat4(1.0f), playerPosition) * playerRotation;

        // Handle camera.
        auto& cameraTransform = registry.get<Components::Transform>(cameraEntity);

        auto cameraPosition = glm::vec3(cameraTransform.transform[3]);
        auto cameraRotation = glm::mat4(glm::mat3(cameraTransform.transform));

        float cameraPitch, cameraYaw, cameraRoll;
        glm::extractEulerAngleYXZ(cameraRotation, cameraYaw, cameraPitch, cameraRoll);

        cameraPitch = glm::degrees(cameraPitch);
        cameraYaw   = glm::degrees(cameraYaw);
        cameraRoll  = glm::degrees(cameraRoll);

        // Camera can orbit around x and y axes.
        if (_mouseButtonToStateMap[Events::MoveEvents::MouseButton::Right] == Events::MoveEvents::InputAction::Pressed) {
            float deltaX = _mousePositionDelta.first * cameraControlComponent.mouseSensitivity;
            float deltaY = _mousePositionDelta.second * cameraControlComponent.mouseSensitivity;

            cameraYaw -= deltaX;
            cameraPitch += deltaY;

            cameraPitch = glm::clamp(cameraPitch, -PITCH_LIMIT, PITCH_LIMIT);
        }

        cameraYaw   = glm::radians(cameraYaw);
        cameraPitch = glm::radians(cameraPitch);

        cameraRotation = glm::eulerAngleYXZ(cameraYaw, cameraPitch, 0.0f);

        cameraControlComponent.currentDistance -= _scrollOffset.second * cameraControlComponent.moveSpeed * deltaTime;
        cameraControlComponent.currentDistance = glm::max(cameraControlComponent.currentDistance, MIN_FOCUS_DISTANCE);

        // Spherical coordinates - why does it differ from Wikipedia? Due to the fact that they provide coordinates in polar angle and not elevation as we do.
        // Therefore to use Wikipedia formula we would need to subtract from pi/2 the pitch and yaw angles. This results in our formula.
        // y & z are swapped due to the fact that we have different coordinate system.
        {
            float x = cameraControlComponent.currentDistance * cos(cameraPitch) * sin(cameraYaw);
            float y = cameraControlComponent.currentDistance * sin(cameraPitch);
            float z = cameraControlComponent.currentDistance * cos(cameraPitch) * cos(cameraYaw);

            cameraPosition = playerPosition + glm::vec3(x, y, z);
        }

        cameraTransform.transform = glm::translate(glm::mat4(1.0f), cameraPosition) * cameraRotation;

        // Slightly above the player.
        glm::vec3 lookTarget = playerPosition + WORLD_UP_VECTOR * 0.5f;

        glm::mat4 view = glm::lookAt(cameraPosition, lookTarget, WORLD_UP_VECTOR);

        cameraComponent.view = std::move(view);
    }

    void MotionControlSystem::Update(float deltaTime, Resources::Scene& scene)
    {
        auto& registry = scene.GetRegistry();

        auto activeCameraView = registry.view<Components::Tags::ActiveCamera>();
        if (activeCameraView.empty()) {
            return;
        }

        if (!registry.all_of<Components::Camera, Components::CameraControl, Components::Transform>(activeCameraView.front())) {
            return;
        }
        auto cameraEntity = activeCameraView.front();

        auto& cameraComponent        = registry.get<Components::Camera>(cameraEntity);
        auto& cameraControlComponent = registry.get<Components::CameraControl>(cameraEntity);

        if (cameraControlComponent.cameraType == Components::CameraControl::CameraType::FirstPerson) {
            updateFirstPersonCamera(registry, cameraEntity, deltaTime);
        } else if (cameraControlComponent.cameraType == Components::CameraControl::CameraType::ThirdPerson) {
            updateThirdPersonCamera(registry, cameraEntity, deltaTime);
        }

        auto [width, height] = _contextResources.GetVulkanResource().GetSwapchainExtent();

        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

        glm::mat4 projection =
            glm::perspective(glm::radians(cameraControlComponent.fov), aspectRatio, cameraControlComponent.nearPlane, cameraControlComponent.farPlane);

        cameraComponent.projection = std::move(projection);

        _mousePositionDelta = {0.f, 0.f};
        _scrollOffset       = {0.f, 0.f};
        _keyToStateMap.clear();
        _mouseButtonToStateMap.clear();
    }

    void MotionControlSystem::onKeyPressed(const Events::KeyPressEvent& event)
    {
        _keyToStateMap[event.key] = event.action;
    }

    void MotionControlSystem::onMousePressed(const Events::MouseButtonPressEvent& event)
    {
        _mouseButtonToStateMap[event.button] = event.action;
    }

    void MotionControlSystem::onMouseMoved(const Events::MouseMoveEvent& event)
    {
        _mousePositionDelta = {event.position.first - _mousePosition.first, _mousePosition.second - event.position.second};

        // Skip first update.
        if (_mousePosition == std::pair<double, double>{0, 0}) {
            _mousePositionDelta = {0, 0};
        }
        _mousePosition = {event.position.first, event.position.second};
    }

    void MotionControlSystem::onMouseScrolled(const Events::MouseScrollEvent& event)
    {
        _scrollOffset = event.scrollOffset;
    }

} // namespace Prism::Systems
