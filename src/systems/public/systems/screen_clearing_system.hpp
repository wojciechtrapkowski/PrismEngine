#pragma once

namespace Prism::Systems {
    class ScreenClearingSystem {
      public:
        ScreenClearingSystem() = default;
        ~ScreenClearingSystem() = default;

        ScreenClearingSystem(ScreenClearingSystem &other) = delete;
        ScreenClearingSystem &operator=(ScreenClearingSystem &other) = delete;

        ScreenClearingSystem(ScreenClearingSystem &&other) = delete;
        ScreenClearingSystem &operator=(ScreenClearingSystem &&other) = delete;

        void Initialize();

        void Update();

        void Render();

      private:
    };
}; // namespace Prism::Systems