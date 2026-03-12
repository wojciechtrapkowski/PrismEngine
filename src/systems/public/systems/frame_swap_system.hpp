#pragma once

#include "resources/context_resources.hpp"

namespace Prism::Systems
{
    class FrameSwapSystem
    {
    public:
        FrameSwapSystem(Resources::ContextResources& contextResources);
        ~FrameSwapSystem() = default;

        FrameSwapSystem(FrameSwapSystem& other)            = delete;
        FrameSwapSystem& operator=(FrameSwapSystem& other) = delete;

        FrameSwapSystem(FrameSwapSystem&& other)            = delete;
        FrameSwapSystem& operator=(FrameSwapSystem&& other) = delete;

        void Update(float deltaTime);

    private:
        Resources::ContextResources& _contextResources;
    };
}; // namespace Prism::Systems