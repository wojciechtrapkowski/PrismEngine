#include "systems/present_system.hpp"

#include "utils/opengl_debug.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace Prism::Systems {
    void PresentSystem::Initialize() {

    };

    void PresentSystem::Update() {

    };

    void PresentSystem::Render(GLFWwindow *window) {
        GLCheck(glfwSwapBuffers(window));
    }
} // namespace Prism::Systems