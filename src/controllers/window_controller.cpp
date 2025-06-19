#include "controllers/window_controller.hpp"

#include "utils/opengl_debug.hpp"

#include <iostream>
#include <stdexcept>

namespace /* anonymous */ {
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;

    void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
        // Make this as event.
        GLCheck(glViewport(0, 0, width, height));
        std::cout << "Resized window  to " << width << " " << height
                  << std::endl;
    }

} // namespace

namespace Prism::Controllers {
    void WindowController::Initialize() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


        m_Window =
            std::unique_ptr<GLFWwindow, GLFWwindowDeleter>(glfwCreateWindow(
                SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr));

        if (!m_Window) {
            throw std::runtime_error("Failed to create GLFW window");
            glfwTerminate();
        }

        glfwMakeContextCurrent(m_Window.get());
        glfwSetFramebufferSizeCallback(m_Window.get(), framebufferSizeCallback);
        glfwSetInputMode(m_Window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}; // namespace Prism::Controllers