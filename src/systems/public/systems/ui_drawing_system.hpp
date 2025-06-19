#pragma once

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

#include "ui/main_dock_ui.hpp"
#include "ui/menu_bar_ui.hpp"
#include "ui/scene_hierarchy_ui.hpp"

namespace Prism::Systems {
    class UIDrawingSystem {
      public:
        UIDrawingSystem(Resources::ContextResources &contextResources);
        ~UIDrawingSystem() = default;

        UIDrawingSystem(const UIDrawingSystem &) = delete;
        UIDrawingSystem &operator=(const UIDrawingSystem &) = delete;

        UIDrawingSystem(UIDrawingSystem &&) = delete;
        UIDrawingSystem &operator=(UIDrawingSystem &&) = delete;

        void Initialize();

        void Update(float deltaTime, Resources::Scene &scene);

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        Resources::ContextResources &m_contextResources;

        UI::MainDockUI mainDockUI;
        UI::MenuBarUI menuBarUI;
        UI::SceneHierarchyUI sceneHierarchyUI;
    };
} // namespace Prism::Systems