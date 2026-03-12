#pragma once

#include "resources/scene.hpp"

#include "resources/context_resources.hpp"

#include <entt/entt.hpp>

#include "events/app_events.hpp"

namespace Prism::Context
{
    struct Context
    {
        Context();
        ~Context() = default;

        Context(Context&& other)      = delete;
        Context& operator=(Context&&) = delete;

        Context(Context& other)      = delete;
        Context& operator=(Context&) = delete;

        void RunEngine();

    private:
        void onWindowClose(Events::WindowCloseEvent& event);

        Resources::ContextResources _contextResources;

        bool                    _isRunning = true;
        entt::scoped_connection _windowCloseEventConnection;

        entt::registry _registry;
    };
}; // namespace Prism::Context