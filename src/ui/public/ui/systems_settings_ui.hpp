#pragma once

#include "resources/context_resources.hpp"
#include "resources/scene.hpp"

namespace Prism::UI
{
    class SystemsSettingsUI
    {
    public:
        SystemsSettingsUI(Resources::ContextResources& contextResources);
        ~SystemsSettingsUI() = default;

        SystemsSettingsUI(const SystemsSettingsUI&)            = delete;
        SystemsSettingsUI& operator=(const SystemsSettingsUI&) = delete;

        SystemsSettingsUI(SystemsSettingsUI&&)            = delete;
        SystemsSettingsUI& operator=(SystemsSettingsUI&&) = delete;

        void Update(float deltaTime, Resources::Scene& scene);

    private:
        Resources::ContextResources& _contextResources;
    };
} // namespace Prism::UI