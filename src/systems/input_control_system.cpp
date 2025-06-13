#include "systems/input_control_system.hpp"

namespace Prism::Systems {
    namespace {
        void processInput(GLFWwindow *window) {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);
        }
    }; // namespace

    void InputControlSystem::Initialize() {

    };

    void InputControlSystem::Update(GLFWwindow *window) {
        glfwPollEvents();
        processInput(window);
    };
} // namespace Prism::Systems