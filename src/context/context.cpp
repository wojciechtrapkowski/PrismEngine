#include "context/context.hpp"

#include "controllers/window_controller.hpp"

#include "loaders/glad_loader.hpp"
#include "loaders/imgui_loader.hpp"
#include "loaders/mesh_loader.hpp"
#include "loaders/shader_loader.hpp"

#include "systems/camera_creation_system.hpp"
#include "systems/event_poll_system.hpp"
#include "systems/fps_motion_control_system.hpp"
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

        Systems::InputControlSystem inputControlSystem{m_contextResources};
        Systems::EventPollSystem eventPollSystem{m_contextResources};
        Systems::CameraCreationSystem cameraCreationSystem{m_contextResources};
        Systems::FpsMotionControlSystem fpsMotionControlSystem{
            m_contextResources};

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
        auto backpackId = std::hash<const char *>{}("MeshResources/Backpack");

        scene.AddNewMesh(backpackId, std::move(backpackModel));

        inputControlSystem.Initialize();
        eventPollSystem.Initialize();
        fpsMotionControlSystem.Initialize();
        cameraCreationSystem.Initialize();

        screenClearingSystem.Initialize();
        meshDrawingSystem.Initialize();
        imGuiDrawingSystem.Initialize();
        presentSystem.Initialize();

        float deltaTime = 0.0f;
        float lastFrameTime = 0.0f;

        // TODO: Add delta time to each interface

        while (!glfwWindowShouldClose(window)) {
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrameTime;
            lastFrameTime = currentFrame;

            eventPollSystem.Update();
            inputControlSystem.Update(window);

            screenClearingSystem.Update();
            meshDrawingSystem.Update();
            imGuiDrawingSystem.Update();
            presentSystem.Update();

            cameraCreationSystem.Update(scene);
            fpsMotionControlSystem.Update(deltaTime, scene);

            screenClearingSystem.Render();
            meshDrawingSystem.Render(scene);
            imGuiDrawingSystem.Render();
            presentSystem.Render(window);
        }


        glfwTerminate();
    }
}; // namespace Prism::Context