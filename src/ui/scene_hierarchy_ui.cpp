#include "ui/scene_hierarchy_ui.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include "components/mesh.hpp"
#include "components/tags.hpp"
#include "components/transform.hpp"

namespace Prism::UI {
    namespace {
        void renderTransformComponent(entt::registry &registry,
                                      entt::entity entity) {
            if (!registry.all_of<Components::Transform>(entity))
                return;

            if (ImGui::TreeNode("Transform")) {
                auto &transform =
                    registry.get<Components::Transform>(entity).transform;

                if (ImGui::BeginTable("TransformMatrix", 4,
                                      ImGuiTableFlags_Borders |
                                          ImGuiTableFlags_RowBg)) {

                    ImGui::TableSetupColumn("X");
                    ImGui::TableSetupColumn("Y");
                    ImGui::TableSetupColumn("Z");
                    ImGui::TableSetupColumn("W");
                    ImGui::TableHeadersRow();

                    for (int row = 0; row < 4; ++row) {
                        ImGui::TableNextRow();
                        for (int col = 0; col < 4; ++col) {
                            ImGui::TableSetColumnIndex(col);
                            std::string label = "##M_" + std::to_string(row) +
                                                "_" + std::to_string(col);
                            ImGui::InputFloat(label.c_str(),
                                              &transform[row][col], 0.1f, 1.0f,
                                              "%.3f");
                        }
                    }

                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
        }

        void renderMeshNode(entt::registry &registry,
                            const entt::entity &meshNodeEntity) {

            auto &meshComponent =
                registry.get<Components::Mesh>(meshNodeEntity);

            ImGuiTreeNodeFlags nodeFlags =
                ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

            if (registry.all_of<Components::Tags::SelectedNode>(
                    meshNodeEntity)) {
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            }

            bool isOpened =
                ImGui::TreeNodeEx((void *)(intptr_t)meshNodeEntity, nodeFlags,
                                  "%s", meshComponent.name.c_str());

            if (ImGui::IsItemClicked()) {
                auto selectedNodeView =
                    registry.view<Components::Tags::SelectedNode>();
                if (!selectedNodeView.empty()) {
                    auto selectedNodeEntity = selectedNodeView.front();
                    registry.remove<Components::Tags::SelectedNode>(
                        selectedNodeEntity);
                }

                registry.emplace<Components::Tags::SelectedNode>(
                    meshNodeEntity);
            }

            if (isOpened) {
                renderTransformComponent(registry, meshNodeEntity);

                // More components in the future...

                ImGui::TreePop();
            }
        }
    } // namespace

    SceneHierarchyUI::SceneHierarchyUI(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {};

    void SceneHierarchyUI::Update(float deltaTime, Resources::Scene &scene) {
        ImGui::Begin("Scene Hierarchy");

        auto &registry = scene.GetRegistry();
        auto meshView = registry.view<Components::Mesh>();
        for (const auto &meshEntity : meshView) {
            renderMeshNode(registry, meshEntity);
        }

        ImGui::End();
    }
} // namespace Prism::UI