#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace Prism::Systems {
    class PresentSystem {
      public:
        PresentSystem() = default;
        ~PresentSystem() = default;

        PresentSystem(PresentSystem &other) = delete;
        PresentSystem &operator=(PresentSystem &other) = delete;

        PresentSystem(PresentSystem &&other) = delete;
        PresentSystem &operator=(PresentSystem &&other) = delete;

        void Initialize();

        void Update();

        void Render(GLFWwindow *window);

      private:
    };
}; // namespace Prism::Systems