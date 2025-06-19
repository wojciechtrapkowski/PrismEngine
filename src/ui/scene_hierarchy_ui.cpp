#include <imgui.h>
#include <imgui_internal.h>

#include "ui/scene_hierarchy_ui.hpp"

namespace Prism::UI {
    SceneHierarchyUI::SceneHierarchyUI(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {};

    void SceneHierarchyUI::Render(float deltaTime, Resources::Scene &scene) {
        ImGui::Begin("Test");
        ImGui::Text("Test");
        ImGui::End();
    }
} // namespace Prism::UI