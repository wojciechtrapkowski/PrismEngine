#include "systems/gizmo_drawing_system.hpp"

#include "components/camera.hpp"
#include "components/fps_camera_control.hpp"
#include "components/mesh.hpp"
#include "components/tags.hpp"
#include "components/transform.hpp"

#include "utils/opengl_debug.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include "ImGuizmo.h"

#include <filesystem>
#include <iostream>
#include <vector>

namespace Prism::Systems {
    namespace {}

    GizmoDrawingSystem::GizmoDrawingSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {

        m_onKeyPressedConnection =
            m_contextResources.dispatcher.sink<Events::KeyPressEvent>()
                .connect<&GizmoDrawingSystem::onKeyPressed>(this);
    };

    void GizmoDrawingSystem::Initialize() {

    };

    void GizmoDrawingSystem::Update(float deltaTime, Resources::Scene &scene) {

        auto &registry = scene.GetRegistry();

        auto activeCameraView = registry.view<Components::Tags::ActiveCamera>();
        if (activeCameraView.empty()) {
            return;
        }
        auto cameraEntity = activeCameraView.front();

        if (!registry.all_of<Components::Camera>(cameraEntity)) {
            return;
        }

        auto selectedNodeView = registry.view<Components::Tags::SelectedNode>();
        if (selectedNodeView.empty()) {
            return;
        }
        auto selectedNodeEntity = selectedNodeView.front();

        if (!registry.all_of<Components::Transform>(selectedNodeEntity)) {
            return;
        }

        auto &camera = registry.get<Components::Camera>(cameraEntity);

        auto &selectedNodeTransform =
            registry.get<Components::Transform>(selectedNodeEntity);

        int width, height;
        GLCheck(glfwGetFramebufferSize(m_contextResources.window.get(), &width,
                                       &height));

        ImGuizmo::OPERATION imGuizmoOperation = ImGuizmo::OPERATION::SCALE;
        if (m_keyToStateMap[Events::Keys::R] == Events::InputAction::Pressed) {
            imGuizmoOperation = ImGuizmo::OPERATION::ROTATE;
        } else if (m_keyToStateMap[Events::Keys::T] ==
                   Events::InputAction::Pressed) {
            imGuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
        }

        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2((float)width, (float)height));

        ImGui::Begin(
            "InvisibleGizmoWindow", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings);

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::BeginFrame();

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x,
                          viewport->Size.y);

        glm::vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents(
            glm::value_ptr(selectedNodeTransform.transform),
            glm::value_ptr(translation), glm::value_ptr(rotation),
            glm::value_ptr(scale));

        ImGuizmo::Manipulate(glm::value_ptr(camera.view),
                             glm::value_ptr(camera.projection),
                             imGuizmoOperation, ImGuizmo::MODE::WORLD,
                             glm::value_ptr(selectedNodeTransform.transform));

        ImGui::End();

        m_keyToStateMap.clear();
    };

    void GizmoDrawingSystem::Render(float deltaTime, Resources::Scene &scene) {}

    void GizmoDrawingSystem::onKeyPressed(const Events::KeyPressEvent &event) {
        m_keyToStateMap[event.key] = event.action;
    }
} // namespace Prism::Systems