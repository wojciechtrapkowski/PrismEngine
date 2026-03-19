#pragma once

#include <GLFW/glfw3.h>

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

#include "events/move_events.hpp"

namespace Prism::Systems
{
    struct MotionControlSystem
    {
        MotionControlSystem(Resources::ContextResources& contextResources);
        ~MotionControlSystem() = default;

        MotionControlSystem(MotionControlSystem& other)            = delete;
        MotionControlSystem& operator=(MotionControlSystem& other) = delete;

        MotionControlSystem(MotionControlSystem&& other)            = delete;
        MotionControlSystem& operator=(MotionControlSystem&& other) = delete;

        void Update(float deltaTime, Resources::Scene& scene);

    private:
        void onKeyPressed(const Events::KeyPressEvent& event);
        void onMousePressed(const Events::MouseButtonPressEvent& event);
        void onMouseMoved(const Events::MouseMoveEvent& event);
        void onMouseScrolled(const Events::MouseScrollEvent& event);

        void updateFirstPersonCamera(entt::registry& registry, entt::entity cameraEntity, float deltaTime);
        void updateThirdPersonCamera(entt::registry& registry, entt::entity cameraEntity, float deltaTime);

        Resources::ContextResources& _contextResources;

        entt::scoped_connection _onKeyPressedConnection;
        entt::scoped_connection _onMousePressedConnection;
        entt::scoped_connection _onMouseMovementConnection;
        entt::scoped_connection _onMouseScrollConnection;

        std::unordered_map<Events::Keys, Events::InputAction> _keyToStateMap;

        std::unordered_map<Events::MouseButton, Events::InputAction> _mouseButtonToStateMap;

        std::pair<double, double> _mousePosition      = {};
        std::pair<double, double> _mousePositionDelta = {};
        std::pair<double, double> _scrollOffset       = {};
    };
}; // namespace Prism::Systems