#include <imgui.h>
#include <imgui_internal.h>

#include "ui/menu_bar_ui.hpp"

namespace Prism::UI {
    MenuBarUI::MenuBarUI(Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {}

    void MenuBarUI::Render(float deltaTime, Resources::Scene &scene) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    // Handle New action
                }
                if (ImGui::MenuItem("Open")) {
                    // Handle Open action
                }
                if (ImGui::MenuItem("Save")) {
                    // Handle Save action
                }
                if (ImGui::MenuItem("Exit")) {
                    // Handle Exit action
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo")) {
                    // Handle Undo
                }
                if (ImGui::MenuItem("Redo")) {
                    // Handle Redo
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    // Show about popup
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
} // namespace Prism::UI