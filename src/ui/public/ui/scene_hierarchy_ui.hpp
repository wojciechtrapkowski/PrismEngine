#pragma once

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::UI {
    class SceneHierarchyUI {
      public:
        SceneHierarchyUI(Resources::ContextResources &contextResources);
        ~SceneHierarchyUI() = default;

        SceneHierarchyUI(const SceneHierarchyUI &) = delete;
        SceneHierarchyUI &operator=(const SceneHierarchyUI &) = delete;

        SceneHierarchyUI(SceneHierarchyUI &&) = delete;
        SceneHierarchyUI &operator=(SceneHierarchyUI &&) = delete;

        void Update(float deltaTime, Resources::Scene &scene);

      private:
        Resources::ContextResources &m_contextResources;
    };
} // namespace Prism::UI