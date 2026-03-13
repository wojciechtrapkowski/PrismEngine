#pragma once

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::UI
{
    class SceneHierarchyUI
    {
    public:
        SceneHierarchyUI(Resources::ContextResources& contextResources);
        ~SceneHierarchyUI() = default;

        SceneHierarchyUI(const SceneHierarchyUI&)            = delete;
        SceneHierarchyUI& operator=(const SceneHierarchyUI&) = delete;

        SceneHierarchyUI(SceneHierarchyUI&&)            = delete;
        SceneHierarchyUI& operator=(SceneHierarchyUI&&) = delete;

        void Update(float deltaTime, Resources::Scene& scene);

    private:
        static constexpr auto ENTITY_RENAME_BUFFER_SIZE = 256;

        Resources::ContextResources& m_contextResources;

        entt::entity      _renamingEntity     = entt::null;
        std::vector<char> _entityRenameBuffer = std::vector<char>(ENTITY_RENAME_BUFFER_SIZE, '\0');
    };
} // namespace Prism::UI