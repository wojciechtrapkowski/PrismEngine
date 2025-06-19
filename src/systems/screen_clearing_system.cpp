#include "systems/screen_clearing_system.hpp"

#include "utils/opengl_debug.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>

namespace Prism::Systems {
    namespace {
        ImVec4 CLEAR_COLOR = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);
    }

    ScreenClearingSystem::ScreenClearingSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {};

    void ScreenClearingSystem::Initialize() {
        // Nothing.
    };

    void ScreenClearingSystem::Update(float deltaTime,
                                      Resources::Scene &scene) {
        // Nothing.
    };

    void ScreenClearingSystem::Render(float deltaTime,
                                      Resources::Scene &scene) {
        GLCheck(glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z,
                             CLEAR_COLOR.w));

        GLCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }
} // namespace Prism::Systems