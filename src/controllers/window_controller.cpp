#include "window_controller.hpp"

#include <stdexcept>

namespace /* anonymous */ {
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;

    void framebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        // make sure the viewport matches the new window dimensions; note that width and 
        // height will be significantly larger than specified on retina displays.
        glViewport(0, 0, width, height);
    }
}

namespace Prism::Controllers {
    void WindowController::Initialize() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif

        m_Window = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>(
            glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr)
        );

        if (!m_Window)
        {
            throw std::runtime_error("Failed to create GLFW window");
            glfwTerminate();
        }

        glfwMakeContextCurrent(m_Window.get());
        glfwSetFramebufferSizeCallback(m_Window.get(), framebufferSizeCallback);
    }
};