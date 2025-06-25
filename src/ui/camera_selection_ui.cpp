#include <imgui.h>
#include <imgui_internal.h>

#include "ui/camera_selection_ui.hpp"

#include "components/fps_camera_control.hpp"
#include "components/tags.hpp"

namespace Prism::UI {
    namespace {
        constexpr auto FPS_CAMERA_NAME = "FPS Camera";

        struct CameraType {
            const char *name;
            std::function<bool(entt::registry &, entt::entity)> checkIfActive;
            std::function<void(entt::registry &)> makeActive;
        };
    } // namespace

    CameraSelectionUI::CameraSelectionUI(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {}

    void CameraSelectionUI::Update(float deltaTime, Resources::Scene &scene) {
        ImGui::Begin("Camera selection");

        auto &registry = scene.GetRegistry();
        auto activeCameraView = registry.view<Components::Tags::ActiveCamera>();
        if (activeCameraView.empty()) {
            ImGui::End();
            return;
        }
        auto activeCameraEntity = activeCameraView.front();

        std::vector<CameraType> availableCameraTypes;

        if (!registry.view<Components::FpsCameraControl>().empty()) {
            availableCameraTypes.push_back(
                {FPS_CAMERA_NAME,
                 [](entt::registry &registry, entt::entity e) {
                     return registry.all_of<Components::FpsCameraControl>(e);
                 },
                 [](entt::registry &registry) {
                     auto fpsCameraView =
                         registry.view<Components::FpsCameraControl>();
                     auto fpsCameraEntity = fpsCameraView.front();
                     registry.emplace<Components::Tags::ActiveCamera>(
                         fpsCameraEntity);
                 }});
        }

        if (availableCameraTypes.empty()) {
            return;
        }

        const char *previewValue = availableCameraTypes.front().name;

        if (ImGui::BeginCombo("Camera Type", previewValue)) {
            for (int i = 0; i < availableCameraTypes.size(); ++i) {
                bool isSelected = (availableCameraTypes[i].checkIfActive(
                    registry, activeCameraEntity));

                if (ImGui::Selectable(availableCameraTypes[i].name,
                                      isSelected)) {
                    registry.remove<Components::Tags::ActiveCamera>(
                        activeCameraEntity);

                    availableCameraTypes[i].makeActive(registry);
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::End();
    }
} // namespace Prism::UI