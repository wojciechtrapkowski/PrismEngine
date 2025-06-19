#include "systems/imgui_drawing_system.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace Prism::Systems {
    ImGuiDrawingSystem::ImGuiDrawingSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {};

    void ImGuiDrawingSystem::Initialize() {

    };

    void ImGuiDrawingSystem::Update(float deltaTime, Resources::Scene &scene) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // TEST
        ImGui::Begin("Another Window");
        ImGui::Text("Hello from another window!");
        ImGui::End();
    };

    void ImGuiDrawingSystem::Render(float deltaTime, Resources::Scene &scene) {


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
} // namespace Prism::Systems