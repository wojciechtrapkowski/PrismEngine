#include "ui/scene_hierarchy_ui.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include "components/mesh.hpp"
#include "components/tags.hpp"
#include "components/transform.hpp"
#include "components/name.hpp"

#include <format>

namespace Prism::UI
{
    namespace
    {
        void renderTransformComponent(entt::registry& registry, entt::entity entity)
        {
            if (!registry.all_of<Components::Transform>(entity))
                return;

            auto isOpened = ImGui::TreeNode("Transform");

            ImGui::OpenPopupOnItemClick("##TransformContext", ImGuiPopupFlags_MouseButtonRight);
            if (ImGui::BeginPopup("##TransformContext")) {
                if (ImGui::MenuItem("Remove Transform")) {
                    registry.remove<Components::Transform>(entity);
                }
                ImGui::EndPopup();
            }

            if (isOpened) {
                // Because user could delete Transform component, we need to check if it still exists.
                auto transformPtr = registry.try_get<Components::Transform>(entity);
                if (!transformPtr) {
                    ImGui::TreePop();
                    return;
                }
                auto& transform = transformPtr->transform;

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

        void renderMeshComponent(
            const std::vector<std::reference_wrapper<Resources::MeshResource>>& availableMeshResources, entt::registry& registry, entt::entity entity)
        {
            if (!registry.all_of<Components::Mesh>(entity))
                return;

            auto isOpened = ImGui::TreeNode("Mesh");

            ImGui::OpenPopupOnItemClick("##MeshContext", ImGuiPopupFlags_MouseButtonRight);
            if (ImGui::BeginPopup("##MeshContext")) {
                if (ImGui::MenuItem("Remove Mesh")) {
                    registry.remove<Components::Mesh>(entity);
                }
                ImGui::EndPopup();
            }

            if (isOpened) {
                // Because user could delete Mesh component, we need to check if it still exists.
                auto meshPtr = registry.try_get<Components::Mesh>(entity);
                if (!meshPtr) {
                    ImGui::TreePop();
                    return;
                }
                auto& mesh = *meshPtr;

                auto currentMeshIndex = std::find_if(
                                            availableMeshResources.begin(),
                                            availableMeshResources.end(),
                                            [&mesh](const Resources::MeshResource& meshResource) { return meshResource.GetID() == mesh.resourceId; }) -
                                        availableMeshResources.begin();

                std::string previewLabel = [&]() {
                    if (currentMeshIndex < 0 || currentMeshIndex >= availableMeshResources.size()) {
                        return std::string("None");
                    }
                    return std::format("Mesh Name: {}", availableMeshResources[currentMeshIndex].get().GetName());
                }();

                if (ImGui::BeginCombo("##MeshResource", previewLabel.c_str())) {
                    for (const auto& meshResourceRef : availableMeshResources) {
                        auto&       meshResource = meshResourceRef.get();
                        std::string label        = std::format("Mesh Name: {}", meshResource.GetName());
                        const bool  isSelected   = (meshResource.GetID() == mesh.resourceId);

                        if (ImGui::Selectable(label.c_str(), isSelected)) {
                            if (!meshResource.GetID()) {
                                continue;
                            }
                            mesh.resourceId = meshResource.GetID().value();
                        }

                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }

                ImGui::TreePop();
            }
        }

        void renderAddingNewComponents(entt::registry& registry, entt::entity entity)
        {
            auto& name = registry.get<Components::Name>(entity);

            ImGui::OpenPopupOnItemClick(std::format("{}_popup", name.name).c_str(), ImGuiPopupFlags_MouseButtonRight);

            if (ImGui::BeginPopup(std::format("{}_popup", name.name).c_str())) {
                if (ImGui::BeginMenu("Add new component")) {
                    if (ImGui::MenuItem("Transform")) {
                        registry.emplace_or_replace<Components::Transform>(entity);
                    }
                    if (ImGui::MenuItem("Mesh")) {
                        registry.emplace_or_replace<Components::Mesh>(entity);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
        }
    } // namespace

    SceneHierarchyUI::SceneHierarchyUI(Resources::ContextResources& contextResources) : _contextResources(contextResources){};

    void SceneHierarchyUI::Update(float deltaTime, Resources::Scene& scene)
    {
        ImGui::Begin("Scene Hierarchy");

        auto& registry    = scene.GetRegistry();
        auto& meshStorage = scene.GetMeshStorage();

        std::vector<std::reference_wrapper<Resources::MeshResource>> availableMeshResources;
        for (auto it = meshStorage.begin<Resources::MeshResource>(); it != meshStorage.end<Resources::MeshResource>(); ++it) {
            availableMeshResources.push_back((*it).second);
        }

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

                bool confirmRename = false;

                if (ImGui::InputText(
                        "##rename",
                        _entityRenameBuffer.data(),
                        _entityRenameBuffer.size(),
                        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                {
                    confirmRename |= true; // confirm on Enter
                }
                if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    confirmRename |= true; // confirm on click outside
                }

                if (confirmRename) {
                    name.name       = _entityRenameBuffer.data();
                    _renamingEntity = entt::null;
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

            renderAddingNewComponents(registry, entity);

            if (isOpened) {
                renderTransformComponent(registry, entity);
                renderMeshComponent(availableMeshResources, registry, entity);

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