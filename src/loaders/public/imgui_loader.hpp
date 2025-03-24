#pragma once

#include <GLFW/glfw3.h>

namespace Prism::Loaders {
    struct ImGuiLoader {
        using result_type = void;

        ImGuiLoader() = default;
        ~ImGuiLoader() = default;

        ImGuiLoader(ImGuiLoader&& other) = delete;
        ImGuiLoader& operator=(ImGuiLoader&&) = delete;
        
        ImGuiLoader(ImGuiLoader& other) = delete;
        ImGuiLoader& operator=(ImGuiLoader&) = delete;

        result_type operator()(GLFWwindow* window) const;
    };
};