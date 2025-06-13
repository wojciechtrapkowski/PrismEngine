#include "systems/present_system.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace Prism::Systems {
    void PresentSystem::Initialize() {

    };

    void PresentSystem::Update() {

    };

    void PresentSystem::Render(GLFWwindow *window) { glfwSwapBuffers(window); }
} // namespace Prism::Systems