#pragma once

#include "resources/resource.hpp"

#include "loaders/window_loader.hpp"

#include <entt/entt.hpp>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace Prism::Resources {
    struct ContextResources : ResourceImpl<ContextResources> {
        std::unique_ptr<GLFWwindow, Loaders::WindowLoader::GLFWwindowDeleter>
            window;
        entt::dispatcher dispatcher;
    };
}; // namespace Prism::Resources