#pragma once

#include "systems/event_poll_system.hpp"
#include "systems/input_control_system.hpp"
#include "systems/window_resize_system.hpp"
#include "systems/frame_swap_system.hpp"

#include "resources/context_resources.hpp"

namespace Prism::Managers
{
    class ContextUpdateSystemsManager
    {
    public:
        ContextUpdateSystemsManager(Resources::ContextResources& contextResources);

        ~ContextUpdateSystemsManager() = default;

        ContextUpdateSystemsManager(const ContextUpdateSystemsManager&)            = delete;
        ContextUpdateSystemsManager& operator=(const ContextUpdateSystemsManager&) = delete;

        ContextUpdateSystemsManager(ContextUpdateSystemsManager&&)            = delete;
        ContextUpdateSystemsManager& operator=(ContextUpdateSystemsManager&&) = delete;

        void Update(float deltaTime);

    private:
        Resources::ContextResources& _contextResources;

        Systems::EventPollSystem    _eventPollSystem;
        Systems::InputControlSystem _inputControlSystem;
        Systems::WindowResizeSystem _windowResizeSystem;
        Systems::FrameSwapSystem    _frameSwapSystem;
    };

} // namespace Prism::Managers