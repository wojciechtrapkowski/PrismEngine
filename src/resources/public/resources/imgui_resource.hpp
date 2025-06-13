#pragma once

namespace Prism::Resources {
    struct ImGuiResource {
        ImGuiResource() = default;
        ~ImGuiResource();

        ImGuiResource(ImGuiResource &other) = delete;
        ImGuiResource &operator=(ImGuiResource &other) = delete;

        ImGuiResource(ImGuiResource &&other) = default;
        ImGuiResource &operator=(ImGuiResource &&other) = default;
    };
}; // namespace Prism::Resources