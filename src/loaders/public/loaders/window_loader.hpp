#pragma once

#include <memory>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace Prism::Loaders {
    struct WindowLoader {
        struct GLFWwindowDeleter {
            void operator()(GLFWwindow *window) {
                if (window) {
                    glfwDestroyWindow(window);
                }
            }
        };

        using result_type = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

        WindowLoader() = default;
        ~WindowLoader() = default;

        WindowLoader(WindowLoader &&other) = default;
        WindowLoader &operator=(WindowLoader &&) = default;

        WindowLoader(WindowLoader &other) = delete;
        WindowLoader &operator=(WindowLoader &) = delete;

        result_type operator()() const;
    };
}; // namespace Prism::Loaders