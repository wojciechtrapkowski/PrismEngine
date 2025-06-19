#include "context/context.hpp"

#include "loaders/glad_loader.hpp"
#include "loaders/imgui_loader.hpp"
#include "loaders/mesh_loader.hpp"
#include "loaders/shader_loader.hpp"
#include "loaders/window_loader.hpp"

#include "systems/camera_creation_system.hpp"
#include "systems/common_uniform_update_system.hpp"
#include "systems/event_poll_system.hpp"
#include "systems/fps_motion_control_system.hpp"
#include "systems/input_control_system.hpp"

#include "events/window_resize_event.hpp"

#include "managers/scene_draw_systems_manager.hpp"
#include "managers/scene_update_systems_manager.hpp"

#include "resources/scene.hpp"

#include <iostream>

namespace Prism::Context {
    namespace {
        void printDebugInfo() {
            const GLubyte *version = glGetString(GL_VERSION);
            std::cout << "OpenGL Version: " << version << std::endl;
        }
    } // namespace

    void Context::RunEngine() {
        Loaders::WindowLoader windowLoader;

        {
            auto window = windowLoader();
            m_contextResources.window = std::move(window);
        }

        auto *window = m_contextResources.window.get();

        Loaders::GladLoader gladLoader;
        gladLoader();

        Loaders::ImGuiLoader imGuiLoader;
        auto imguiResource = imGuiLoader(window);


        Systems::EventPollSystem eventPollSystem{m_contextResources};

        Systems::InputControlSystem inputControlSystem{m_contextResources};

        Managers::SceneDrawSystemsManager sceneDrawSystemsManager{
            m_contextResources};

        Managers::SceneUpdateSystemsManager sceneUpdateSystemsManager{
            m_contextResources};

        Loaders::MeshLoader meshLoader{};

        Resources::Scene scene{};

        auto backpackModelOpt = meshLoader("backpack.obj");
        if (!backpackModelOpt) {
            std::cerr << "Couldn't load backpack model!" << std::endl;
        } else {
            auto &backpackModel = *backpackModelOpt;
            auto backpackId =
                std::hash<const char *>{}("MeshResources/Backpack");
            scene.AddNewMesh(backpackId, std::move(backpackModel));

            std::cout << "Loaded backpack model!" << std::endl;
        }

        // auto cubeModelOpt = meshLoader("cube.obj");
        // if (!cubeModelOpt) {
        //     std::cerr << "Couldn't load cube model!" << std::endl;
        // }
        // else {
        //     auto &cubeModel = *cubeModelOpt;
        //     auto cubeId = std::hash<const char *>{}("MeshResources/Cube");
        //     scene.AddNewMesh(cubeId, std::move(cubeModel));
        // }
        //

        eventPollSystem.Initialize();
        inputControlSystem.Initialize();

        sceneUpdateSystemsManager.Initialize();

        sceneDrawSystemsManager.Initialize();

        float deltaTime = 0.0f;
        float lastFrameTime = 0.0f;

        // In the future we have to add debug & release mode.
#ifdef DEBUG
        printDebugInfo();
#endif

        while (!glfwWindowShouldClose(window)) {
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrameTime;
            lastFrameTime = currentFrame;

            eventPollSystem.Update(deltaTime);

            inputControlSystem.Update(deltaTime);

            sceneUpdateSystemsManager.Update(deltaTime, scene);

            sceneDrawSystemsManager.Update(deltaTime, scene);

            sceneDrawSystemsManager.Render(deltaTime, scene);
        }


        glfwTerminate();
    }
}; // namespace Prism::Context