#include "systems/present_system.hpp"

#include "utils/opengl_debug.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace Prism::Systems {
    PresentSystem::PresentSystem(Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {}

    void PresentSystem::Initialize() {

    };

    void PresentSystem::Update(float deltaTime, Resources::Scene &scene) {

    };

    void PresentSystem::Render(float deltaTime, Resources::Scene &scene) {
        auto window = m_contextResources.window.get();
        GLCheck(glfwSwapBuffers(window));
    }
} // namespace Prism::Systems