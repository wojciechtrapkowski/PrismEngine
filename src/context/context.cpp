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
#include "systems/mesh_loading_system.hpp"

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

        Managers::SceneUpdateSystemsManager sceneUpdateSystemsManager{_contextResources};

        Managers::SceneDrawSystemsManager sceneDrawSystemsManager{_contextResources};

        Resources::Scene scene{};

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

                sceneDrawSystemsManager.Update(deltaTime, scene);

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