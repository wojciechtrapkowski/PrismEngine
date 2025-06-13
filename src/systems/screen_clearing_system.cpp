#include "systems/screen_clearing_system.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>

namespace Prism::Systems {
    namespace {
        ImVec4 CLEAR_COLOR = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);
    }

    void ScreenClearingSystem::Initialize() {
        // Nothing.
    };

    void ScreenClearingSystem::Update() {
        // Nothing.
    };

    void ScreenClearingSystem::Render() {
        glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z,
                     CLEAR_COLOR.w);

        glClear(GL_COLOR_BUFFER_BIT);
    }
} // namespace Prism::Systems