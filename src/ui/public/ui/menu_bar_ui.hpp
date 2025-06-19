#pragma once

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::UI {
    class MenuBarUI {
      public:
        MenuBarUI(Resources::ContextResources &contextResources);
        ~MenuBarUI() = default;

        MenuBarUI(const MenuBarUI &) = delete;
        MenuBarUI &operator=(const MenuBarUI &) = delete;

        MenuBarUI(MenuBarUI &&) = delete;
        MenuBarUI &operator=(MenuBarUI &&) = delete;

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        Resources::ContextResources &m_contextResources;
    };
} // namespace Prism::UI