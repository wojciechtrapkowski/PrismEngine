#include "context/context.hpp"

#include "loaders/imgui_loader.hpp"
#include "loaders/mesh_loader.hpp"
#include "loaders/vulkan_loader.hpp"
#include "loaders/window_loader.hpp"

#include "managers/context_update_systems_manager.hpp"
#include "managers/scene_draw_systems_manager.hpp"
#include "managers/scene_update_systems_manager.hpp"

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

#include <format>
#include <iostream>

namespace Prism::Context
{
    namespace
    {
        struct FPSCounter
        {
            size_t frames   = 0;
            double lastTime = glfwGetTime();
        };

        Resources::ContextResources createContextResources()
        {
            Loaders::WindowLoader windowLoader;
            auto                  windowResource = windowLoader();

            Loaders::VulkanLoader vulkanLoader;
            auto                  vulkanResource = vulkanLoader(windowResource);

            if (!vulkanResource) {
                throw std::runtime_error("Couldn't load Vulkan!");
            }

            Loaders::ImGuiLoader imGuiLoader;
            auto                 imguiResource = imGuiLoader(windowResource, vulkanResource.value());

            if (!imguiResource) {
                throw std::runtime_error("Couldn't load imgui!");
            }

            return {std::move(windowResource), std::move(*vulkanResource), std::move(*imguiResource)};
        }
    } // namespace

    Context::Context() : _contextResources{createContextResources()}
    {
        _windowCloseEventConnection = _contextResources.GetDispatcher().sink<Events::WindowCloseEvent>().connect<&Context::onWindowClose>(this);
    }

    void Context::RunEngine()
    {
        Managers::ContextUpdateSystemsManager contextUpdateSystemsManager{_contextResources};

        Managers::SceneDrawSystemsManager sceneDrawSystemsManager{_contextResources};

        Managers::SceneUpdateSystemsManager sceneUpdateSystemsManager{_contextResources};

        Resources::Scene scene{};

        Loaders::MeshLoader meshLoader{};

        Resources::VkStagingBufferResource stagingBuffer{_contextResources.GetVulkanResource().GetVmaAllocator()};

        auto backpackModelOpt = meshLoader(_contextResources.GetVulkanResource(), stagingBuffer, "backpack.obj");
        if (!backpackModelOpt) {
            std::cerr << "Couldn't load backpack model!" << std::endl;
        } else {
            auto& backpackModel = *backpackModelOpt;
            auto  backpackId    = std::hash<const char*>{}("MeshResources/Backpack");
            scene.AddNewMesh(backpackId, "Backpack", std::move(backpackModel));

            std::cout << "Loaded backpack model!" << std::endl;
        }

        // auto cubeModelOpt = meshLoader("cube.obj");
        // if (!cubeModelOpt) {
        //     std::cerr << "Couldn't load cube model!" << std::endl;
        // } else {
        //     auto &cubeModel = *cubeModelOpt;
        //     auto cubeId = std::hash<const char *>{}("MeshResources/Cube");
        //     scene.AddNewMesh(cubeId, "Cube", std::move(cubeModel));
        // }

        float deltaTime     = 0.0f;
        float lastFrameTime = 0.0f;

        FPSCounter fpsCounter{};

        // Scope for cleanup
        {
            auto& windowResource = _contextResources.GetWindowResource();
            auto& vulkanResource = _contextResources.GetVulkanResource();

            while (_isRunning) {
                float currentTime = glfwGetTime();
                deltaTime         = currentTime - lastFrameTime;
                lastFrameTime     = currentTime;

                contextUpdateSystemsManager.Update(deltaTime);

                sceneUpdateSystemsManager.Update(deltaTime, scene);

                sceneDrawSystemsManager.Update(deltaTime, scene, stagingBuffer);

                // Display FPS counter every second
                fpsCounter.frames++;
                if (currentTime - fpsCounter.lastTime >= 1.0) {
                    std::string title = std::format("FPS: {}", fpsCounter.frames);
                    glfwSetWindowTitle(windowResource.GetWindow(), title.c_str());
                    fpsCounter.frames   = 0;
                    fpsCounter.lastTime = currentTime;
                }
            }

            vkDeviceWaitIdle(_contextResources.GetVulkanResource().GetDevice());
        }
    }

    void Context::onWindowClose(Events::WindowCloseEvent& event)
    {
        _isRunning = false;
    }
}; // namespace Prism::Context