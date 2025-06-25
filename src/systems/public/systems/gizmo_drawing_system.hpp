#pragma once

#include "loaders/shader_loader.hpp"

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"
#include "resources/shader_resource.hpp"

#include "events/move_events.hpp"

namespace Prism::Systems {
    class GizmoDrawingSystem {
      public:
        GizmoDrawingSystem(Resources::ContextResources &contextResources);
        ~GizmoDrawingSystem() = default;

        GizmoDrawingSystem(GizmoDrawingSystem &other) = delete;
        GizmoDrawingSystem &operator=(GizmoDrawingSystem &other) = delete;

        GizmoDrawingSystem(GizmoDrawingSystem &&other) = delete;
        GizmoDrawingSystem &operator=(GizmoDrawingSystem &&other) = delete;

        void Initialize();

        void Update(float deltaTime, Resources::Scene &scene);

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        void onKeyPressed(const Events::KeyPressEvent &event);

        Resources::ContextResources &m_contextResources;

        entt::scoped_connection m_onKeyPressedConnection;

        std::unordered_map<Events::Keys, Events::InputAction> m_keyToStateMap;
    };
}; // namespace Prism::Systems