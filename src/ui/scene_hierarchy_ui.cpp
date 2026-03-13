#include "ui/scene_hierarchy_ui.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include "components/mesh.hpp"
#include "components/tags.hpp"
#include "components/transform.hpp"
#include "components/name.hpp"

namespace Prism::UI
{
    namespace
    {
        void renderTransformComponent(entt::registry& registry, entt::entity entity)
        {
            if (!registry.all_of<Components::Transform>(entity))
                return;

            if (ImGui::TreeNode("Transform")) {
                auto& transform = registry.get<Components::Transform>(entity).transform;

                if (ImGui::BeginTable("TransformMatrix", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("X");
                    ImGui::TableSetupColumn("Y");
                    ImGui::TableSetupColumn("Z");
                    ImGui::TableSetupColumn("W");
                    ImGui::TableHeadersRow();

                    for (int row = 0; row < 4; ++row) {
                        ImGui::TableNextRow();
                        for (int col = 0; col < 4; ++col) {
                            ImGui::TableSetColumnIndex(col);
                            std::string label = "##M_" + std::to_string(row) + "_" + std::to_string(col);
                            ImGui::InputFloat(label.c_str(), &transform[row][col], 0.1f, 1.0f, "%.3f");
                        }
                    }

                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
        }
    } // namespace

    SceneHierarchyUI::SceneHierarchyUI(Resources::ContextResources& contextResources) : m_contextResources(contextResources){};

    void SceneHierarchyUI::Update(float deltaTime, Resources::Scene& scene)
    {
        ImGui::Begin("Scene Hierarchy");

        auto& registry = scene.GetRegistry();
        auto  storage  = registry.storage();

        // We will display only entities that have a Name component.
        for (auto entity : registry.view<Components::Name>()) {
            auto& name = registry.get<Components::Name>(entity);

            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (registry.all_of<Components::Tags::SelectedNode>(entity)) {
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            }

            bool isRenaming = (_renamingEntity == entity);
            bool isOpened   = false;

            if (isRenaming) {
                ImGui::SameLine();
                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText(
                        "##rename",
                        _entityRenameBuffer.data(),
                        _entityRenameBuffer.size(),
                        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                {
                    name.name       = _entityRenameBuffer.data();
                    _renamingEntity = entt::null; // confirm on Enter
                }
                if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    name.name       = _entityRenameBuffer.data();
                    _renamingEntity = entt::null; // confirm on click outside
                }
            } else {
                isOpened = ImGui::TreeNodeEx((void*)(intptr_t)entity, nodeFlags, isRenaming ? " " : "%s", name.name.c_str());
            }

            if (ImGui::IsItemClicked()) {
                auto selectedNodeView = registry.view<Components::Tags::SelectedNode>();
                if (!selectedNodeView.empty()) {
                    registry.remove<Components::Tags::SelectedNode>(selectedNodeView.front());
                }
                registry.emplace_or_replace<Components::Tags::SelectedNode>(entity);
            }

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
                _renamingEntity = entity;
                std::snprintf(_entityRenameBuffer.data(), _entityRenameBuffer.size(), "%s", name.name.c_str());
            }

            if (isOpened) {
                if (registry.all_of<Components::Transform>(entity)) {
                    renderTransformComponent(registry, entity);
                }

                ImGui::TreePop();
            }
        }

        if (ImGui::BeginPopupContextWindow("##SceneHierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create new entity")) {
                auto newEntity = registry.create();
                registry.emplace<Components::Name>(newEntity, "New Entity");
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }
} // namespace Prism::UI