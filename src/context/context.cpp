#include "context/context.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "controllers/window_controller.hpp"

#include "loaders/glad_loader.hpp"
#include "loaders/imgui_loader.hpp"
#include "loaders/mesh_loader.hpp"
#include "loaders/shader_loader.hpp"

#include "systems/imgui_drawing_system.hpp"
#include "systems/input_control_system.hpp"
#include "systems/mesh_drawing_system.hpp"
#include "systems/present_system.hpp"
#include "systems/screen_clearing_system.hpp"

#include "resources/scene.hpp"

#include <iostream>

namespace Prism::Context {
    void Context::RunEngine() {

        Controllers::WindowController windowController;
        windowController.Initialize();

        auto window = windowController.GetWindow();

        Loaders::GladLoader gladLoader;
        gladLoader();

        Loaders::ImGuiLoader imGuiLoader;
        auto imguiResource = imGuiLoader(window);

        Loaders::MeshLoader meshLoader{};

        Systems::InputControlSystem inputControlSystem{};

        Systems::ScreenClearingSystem screenClearingSystem{};
        Systems::MeshDrawingSystem meshDrawingSystem{};
        Systems::ImGuiDrawingSystem imGuiDrawingSystem{};
        Systems::PresentSystem presentSystem{};

        Resources::Scene scene{};
        Resources::MeshResource::MeshDescriptor meshDescriptor{
            .vertices = {{glm::vec3{-0.5f, -0.5f, 0.0f}},
                         {glm::vec3{0.5f, -0.5f, 0.0f}},
                         {glm::vec3{0.0f, 0.5f, 0.0f}}},
            .indices = {{0}, {1}, {2}}};


        // scene.AddNewMesh(std::make_unique<Resources::MeshResource>(
        //     std::move(meshDescriptor)));

        auto backpackModelOpt = meshLoader("backpack.obj");
        if (!backpackModelOpt) {
            std::cerr << "Couldn't load backpack model!" << std::endl;
        }
        auto &backpackModel = *backpackModelOpt;

        scene.AddNewMesh(std::move(backpackModel));

        inputControlSystem.Initialize();

        screenClearingSystem.Initialize();
        meshDrawingSystem.Initialize();
        imGuiDrawingSystem.Initialize();
        presentSystem.Initialize();

        while (!glfwWindowShouldClose(window)) {
            inputControlSystem.Update(window);

            screenClearingSystem.Update();
            meshDrawingSystem.Update();
            imGuiDrawingSystem.Update();
            presentSystem.Update();

            screenClearingSystem.Render();
            meshDrawingSystem.Render(scene);
            imGuiDrawingSystem.Render();
            presentSystem.Render(window);
        }


        glfwTerminate();
    }
}; // namespace Prism::Context