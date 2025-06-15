#pragma once

#include <GLFW/glfw3.h>

#include <optional>
#include <utility>

#include "resources/imgui_resource.hpp"

namespace Prism::Loaders {
    struct ImGuiLoader {
        using result_type =
            std::optional<std::unique_ptr<Resources::ImGuiResource>>;

        ImGuiLoader() = default;
        ~ImGuiLoader() = default;

        ImGuiLoader(ImGuiLoader &&other) = default;
        ImGuiLoader &operator=(ImGuiLoader &&) = default;

        ImGuiLoader(ImGuiLoader &other) = delete;
        ImGuiLoader &operator=(ImGuiLoader &) = delete;

        result_type operator()(GLFWwindow *window) const;
    };
}; // namespace Prism::Loaders