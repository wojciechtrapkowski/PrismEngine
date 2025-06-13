#pragma once

namespace Prism::Systems {
    class ImGuiDrawingSystem {
      public:
        ImGuiDrawingSystem() = default;
        ~ImGuiDrawingSystem() = default;

        ImGuiDrawingSystem(ImGuiDrawingSystem &other) = delete;
        ImGuiDrawingSystem &operator=(ImGuiDrawingSystem &other) = delete;

        ImGuiDrawingSystem(ImGuiDrawingSystem &&other) = delete;
        ImGuiDrawingSystem &operator=(ImGuiDrawingSystem &&other) = delete;

        void Initialize();

        void Update();

        void Render();

      private:
    };
}; // namespace Prism::Systems