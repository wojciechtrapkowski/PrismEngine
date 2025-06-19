#include "systems/imgui_drawing_system.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "events/move_events.hpp"

namespace Prism::Systems {
    namespace {
        void ShowMainDockSpace() {
            bool dockspaceOpen = true;

            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar |
                                           ImGuiWindowFlags_NoDocking |
                                           ImGuiWindowFlags_NoBackground;

            windowFlags |= ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoCollapse |
                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

            windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus |
                           ImGuiWindowFlags_NoNavFocus;

            ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                                ImVec2(0.0f, 0.0f));

            ImGui::Begin("Dockspace", &dockspaceOpen, windowFlags);

            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();

            ImGuiID dockspaceId = ImGui::GetID("Dockspace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f),
                             ImGuiDockNodeFlags_PassthruCentralNode);

            ImGui::End();
        }
    } // namespace

    ImGuiDrawingSystem::ImGuiDrawingSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {}

    void ImGuiDrawingSystem::Initialize() {
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    void ImGuiDrawingSystem::Update(float deltaTime, Resources::Scene &scene) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::GetIO().WantCaptureMouse) {
            m_contextResources.dispatcher.clear<Events::MouseMoveEvent>();
        }

        ShowMainDockSpace();

        // TEST
        ImGui::Begin("Test Window");
        ImGui::Text("Hello from a simple window!");
        ImGui::End();
    }

    void ImGuiDrawingSystem::Render(float deltaTime, Resources::Scene &scene) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
} // namespace Prism::Systems