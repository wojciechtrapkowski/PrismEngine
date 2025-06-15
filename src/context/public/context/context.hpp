#pragma once

#include "resources/scene.hpp"

#include "resources/context_resources.hpp"

#include <entt/entt.hpp>

namespace Prism::Context {
    struct Context {
        Context() = default;
        ~Context() = default;

        Context(Context &&other) = delete;
        Context &operator=(Context &&) = delete;

        Context(Context &other) = delete;
        Context &operator=(Context &) = delete;

        void RunEngine();

      private:
        Resources::ContextResources m_contextResources;
        entt::registry m_registry;
    };
}; // namespace Prism::Context