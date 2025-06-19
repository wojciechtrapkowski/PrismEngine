#pragma once

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::UI {
    class MainDockUI {
      public:
        MainDockUI(Resources::ContextResources &contextResources);
        ~MainDockUI() = default;

        MainDockUI(const MainDockUI &) = delete;
        MainDockUI &operator=(const MainDockUI &) = delete;

        MainDockUI(MainDockUI &&) = delete;
        MainDockUI &operator=(MainDockUI &&) = delete;

        void Render(float deltaTime, Resources::Scene &scene);

      private:
        Resources::ContextResources &m_contextResources;
    };
} // namespace Prism::UI