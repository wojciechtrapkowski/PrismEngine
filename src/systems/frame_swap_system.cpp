#include "systems/frame_swap_system.hpp"

#include "imgui.h"

#include "events/move_events.hpp"

namespace Prism::Systems
{
    namespace
    {}; // namespace

    FrameSwapSystem::FrameSwapSystem(Resources::ContextResources& contextResources) : _contextResources(contextResources){};

    void FrameSwapSystem::Update(float deltaTime)
    {
        auto& vulkanResource           = _contextResources.GetVulkanResource();
        auto& temporaryResourceStorage = _contextResources.GetTemporaryResourceStorage();

        vulkanResource.AdvanceFrame();
        temporaryResourceStorage.Clear();
    };
} // namespace Prism::Systems