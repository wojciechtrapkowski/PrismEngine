#include <imgui.h>
#include <imgui_internal.h>

#include "ui/camera_settings_ui.hpp"

#include "components/camera_control.hpp"
#include "components/tags.hpp"

namespace Prism::UI
{
    CameraSettingsUI::CameraSettingsUI(Resources::ContextResources& contextResources) : m_contextResources(contextResources){};

    void CameraSettingsUI::Update(float deltaTime, Resources::Scene& scene)
    {
        ImGui::Begin("Camera settings");

        auto& registry         = scene.GetRegistry();
        auto  activeCameraView = registry.view<Components::Tags::ActiveCamera>();
        if (activeCameraView.empty()) {
            ImGui::End();
            return;
        }
        auto activeCameraEntity = activeCameraView.front();

        auto& cameraControl = registry.get<Components::CameraControl>(activeCameraEntity);

        ImGui::SliderFloat("Mouse sensitivity", &cameraControl.mouseSensitivity, 0.1f, 5.0f);
        ImGui::SliderFloat("Move speed", &cameraControl.moveSpeed, 1.0f, 20.0f);
        ImGui::SliderFloat("FOV", &cameraControl.fov, 45.0f, 90.0f);
        ImGui::SliderFloat("Near plane", &cameraControl.nearPlane, 0.01f, 1.0f);
        ImGui::SliderFloat("Far plane", &cameraControl.farPlane, 10.0f, 10000.0f);

        ImGui::NewLine();
        ImGui::Separator();
        ImGui::NewLine();

        ImGui::BeginChild("Camera selection");

        constexpr const char* cameraTypeNames[] = {"First Person", "Third Person"};
        int                   currentType       = static_cast<int>(cameraControl.cameraType);

        if (ImGui::BeginCombo("Camera Type", cameraTypeNames[currentType])) {
            for (int i = 0; i < IM_ARRAYSIZE(cameraTypeNames); ++i) {
                bool isSelected = (currentType == i);

                if (ImGui::Selectable(cameraTypeNames[i], isSelected)) {
                    cameraControl.cameraType = static_cast<Components::CameraControl::CameraType>(i);
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::EndChild();

        ImGui::End();
    }
} // namespace Prism::UI