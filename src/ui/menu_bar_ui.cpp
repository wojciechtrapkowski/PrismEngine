#include <imgui.h>
#include <imgui_internal.h>

#include "ui/menu_bar_ui.hpp"

namespace Prism::UI {
    MenuBarUI::MenuBarUI(Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {}

    void MenuBarUI::Update(float deltaTime, Resources::Scene &scene) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load Model")) {
                }
                if (ImGui::MenuItem("Open")) {
                }
                if (ImGui::MenuItem("Save")) {
                }
                if (ImGui::MenuItem("Exit")) {
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
} // namespace Prism::UI