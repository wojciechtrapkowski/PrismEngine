#include "loaders/imgui_loader.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stdexcept>

namespace Prism::Loaders {
    ImGuiLoader::result_type ImGuiLoader::operator()(GLFWwindow *window) const {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags =
            ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        return {std::make_unique<Resources::ImGuiResource>()};
    }
}; // namespace Prism::Loaders