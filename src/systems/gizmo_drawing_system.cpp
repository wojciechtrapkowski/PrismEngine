#include "systems/gizmo_drawing_system.hpp"

#include "components/fps_camera_control.hpp"
#include "components/fps_motion_control.hpp"
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
        : m_contextResources(contextResources) {};

    void GizmoDrawingSystem::Initialize() {

    };

    void GizmoDrawingSystem::Update(float deltaTime, Resources::Scene &scene) {

    };

    void GizmoDrawingSystem::Render(float deltaTime, Resources::Scene &scene) {

        auto &registry = scene.GetRegistry();

        auto activeCameraView = registry.view<Components::Tags::ActiveCamera>();
        if (activeCameraView.empty()) {
            return;
        }
        auto cameraEntity = activeCameraView.front();

        if (!registry.all_of<Components::FpsCameraControl,
                             Components::FpsMotionControl>(cameraEntity)) {
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

        auto &cameraSettings =
            registry.get<Components::FpsCameraControl>(cameraEntity);
        auto &cameraControl =
            registry.get<Components::FpsMotionControl>(cameraEntity);

        auto &cameraPosition = cameraControl.cameraPosition;
        auto &cameraForward = cameraControl.cameraForward;
        auto &cameraRight = cameraControl.cameraRight;
        auto &cameraUp = cameraControl.cameraUp;

        glm::mat4 view = glm::lookAt(cameraPosition,
                                     cameraPosition + cameraForward, cameraUp);

        int width, height;
        GLCheck(glfwGetFramebufferSize(m_contextResources.window.get(), &width,
                                       &height));
        glViewport(0, 0, width, height);

        float aspectRatio =
            static_cast<float>(width) / static_cast<float>(height);

        glm::mat4 projection =
            glm::perspective(glm::radians(cameraSettings.fov), aspectRatio,
                             cameraSettings.nearPlane, cameraSettings.farPlane);

        auto &selectedNodeTransform =
            registry.get<Components::Transform>(selectedNodeEntity);

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

        ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                             ImGuizmo::OPERATION::TRANSLATE,
                             ImGuizmo::MODE::WORLD,
                             glm::value_ptr(selectedNodeTransform.transform));

        ImGui::End();
    }
} // namespace Prism::Systems