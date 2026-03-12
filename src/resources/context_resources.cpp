#include "resources/context_resources.hpp"

namespace Prism::Resources
{

    ContextResources::ContextResources(
        Resources::WindowResource&& windowResource, Resources::VulkanResource&& vulkanResource, Resources::ImGuiResource&& imguiResource) :
        _dispatcher{},
        _windowResource(std::move(windowResource)), _vulkanResource(std::move(vulkanResource)), _imguiResource(std::move(imguiResource)), _resourceStorage{},
        _temporaryResourceStorage{}
    {}

} // namespace Prism::Resources