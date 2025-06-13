#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <memory>

namespace /* anonymous */ {
    struct GLFWwindowDeleter {
        void operator()(GLFWwindow *window) {
            if (window) {
                glfwDestroyWindow(window);
            }
        }
    };
} // namespace

namespace Prism::Controllers {
    struct WindowController {
        WindowController() = default;
        ~WindowController() = default;

        WindowController(WindowController &&other) = delete;
        WindowController &operator=(WindowController &&) = delete;

        WindowController(WindowController &other) = delete;
        WindowController &operator=(WindowController &) = delete;

      public:
        void Initialize();

        GLFWwindow *GetWindow() const { return m_Window.get(); }

      private:
        std::unique_ptr<GLFWwindow, GLFWwindowDeleter> m_Window = nullptr;
    };
}; // namespace Prism::Controllers