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

// Testing purposes
#include "components/transform.hpp"
#include "components/mesh.hpp"
#include "components/name.hpp"
#include "components/tags.hpp"

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

        Systems::MeshLoadingSystem meshLoadingSystem{_contextResources};

        Managers::SceneDrawSystemsManager sceneDrawSystemsManager{_contextResources};

        Resources::Scene scene{};

        Loaders::MeshLoader meshLoader{};

        Resources::VkStagingBufferResource stagingBuffer{_contextResources.GetVulkanResource().GetVmaAllocator()};

        // For testing purposes.
        auto backpackModelOpt = meshLoader(_contextResources.GetVulkanResource(), stagingBuffer, "models/backpack.obj");
        if (!backpackModelOpt) {
            std::cerr << "Couldn't load backpack model!" << std::endl;
        } else {
            auto& backpackModel = *backpackModelOpt;
            auto  backpackId    = std::hash<const char*>{}("MeshResources/Backpack");

            auto& meshStorage = scene.GetMeshStorage();
            meshStorage.Insert<Resources::MeshResource>(backpackId, std::move(backpackModel));

            auto& registry = scene.GetRegistry();
            auto  entity   = registry.create();
            registry.emplace<Components::Mesh>(entity, backpackId);
            registry.emplace<Components::Transform>(entity, glm::mat4(1.0f));
            registry.emplace<Components::Name>(entity, "Backpack");
            registry.emplace<Components::Tags::ActivePlayer>(entity);
        }

        auto cubeModelOpt = meshLoader(_contextResources.GetVulkanResource(), stagingBuffer, "models/cube.obj");
        if (!cubeModelOpt) {
            std::cerr << "Couldn't load cube model!" << std::endl;
        } else {
            auto& cubeModel = *cubeModelOpt;
            auto  cubeId    = std::hash<const char*>{}("MeshResources/Cube");

            auto& meshStorage = scene.GetMeshStorage();
            meshStorage.Insert<Resources::MeshResource>(cubeId, std::move(cubeModel));

            auto& registry = scene.GetRegistry();
            auto  entity   = registry.create();
            registry.emplace<Components::Mesh>(entity, cubeId);
            registry.emplace<Components::Transform>(entity, glm::mat4(1.0f));
            registry.emplace<Components::Name>(entity, "Cube");
        }

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

                // ExternalDataLoadingSystemsManager or something like that.
                // Then we can remove scene as a parameter. These value should be stored per context in my opinion.
                // But I still need to think about it.
                meshLoadingSystem.Update(deltaTime, scene, stagingBuffer);

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