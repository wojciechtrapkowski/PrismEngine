#pragma once

#include "resources/resource.hpp"

#include "resources/imgui_resource.hpp"
#include "resources/resource_storage.hpp"
#include "resources/vulkan_resource.hpp"
#include "resources/window_resource.hpp"

#include <entt/entt.hpp>

namespace Prism::Resources
{
    struct ContextResources : ResourceImpl<ContextResources>
    {
        ContextResources(Resources::WindowResource&& windowResource, Resources::VulkanResource&& vulkanResource, Resources::ImGuiResource&& imguiResource);
        ~ContextResources() = default;

        ContextResources(ContextResources& other)            = delete;
        ContextResources& operator=(ContextResources& other) = delete;

        ContextResources(ContextResources&& other)            = default;
        ContextResources& operator=(ContextResources&& other) = default;

        entt::dispatcher& GetDispatcher() { return _dispatcher; }

        Resources::WindowResource& GetWindowResource() { return _windowResource; }

        Resources::VulkanResource& GetVulkanResource() { return _vulkanResource; }

        Resources::ImGuiResource& GetImGuiResource() { return _imguiResource; }

        Resources::ResourceStorage& GetResourceStorage() { return _resourceStorage; }

        // Note: All of the resources in this storage will be purged in the next frame. Use it for resources that are needed only for a single frame.
        Resources::ResourceStorage& GetTemporaryResourceStorage() { return _temporaryResourceStorage; }

    private:
        entt::dispatcher          _dispatcher;
        Resources::WindowResource _windowResource;
        Resources::VulkanResource _vulkanResource;
        Resources::ImGuiResource  _imguiResource;

        Resources::ResourceStorage _resourceStorage;
        Resources::ResourceStorage _temporaryResourceStorage;
    };
}; // namespace Prism::Resources