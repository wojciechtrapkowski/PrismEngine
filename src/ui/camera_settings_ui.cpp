#include <imgui.h>
#include <imgui_internal.h>

#include "ui/camera_settings_ui.hpp"

#include "components/fps_camera_control.hpp"
#include "components/tags.hpp"

namespace Prism::UI {
    CameraSettingsUI::CameraSettingsUI(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {};

    void CameraSettingsUI::Render(float deltaTime, Resources::Scene &scene) {
        ImGui::Begin("Camera settings");

        auto &registry = scene.GetRegistry();
        auto activeCameraView = registry.view<Components::Tags::ActiveCamera>();
        if (activeCameraView.empty()) {
            return;
        }
        auto activeCameraEntity = activeCameraView.front();


        // For now, we support only FPS camera
        bool isFpsCamera =
            registry.all_of<Components::FpsCameraControl>(activeCameraEntity);
        if (!isFpsCamera) {
            return;
        }

        auto &fpsCameraControl =
            registry.get<Components::FpsCameraControl>(activeCameraEntity);

        ImGui::SliderFloat("Mouse sensitivity",
                           &fpsCameraControl.mouseSensitivity, 0.1f, 5.0f);
        ImGui::SliderFloat("Move speed", &fpsCameraControl.moveSpeed, 1.0f,
                           20.0f);
        ImGui::SliderFloat("FOV", &fpsCameraControl.fov, 45.0f, 90.0f);
        ImGui::SliderFloat("Near plane", &fpsCameraControl.nearPlane, 0.01f,
                           1.0f);
        ImGui::SliderFloat("Far plane", &fpsCameraControl.farPlane, 10.0f,
                           10000.0f);


        ImGui::End();
    }
} // namespace Prism::UI