#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace Prism::Systems {
    class InputControlSystem {
      public:
        InputControlSystem() = default;
        ~InputControlSystem() = default;

        InputControlSystem(InputControlSystem &other) = delete;
        InputControlSystem &operator=(InputControlSystem &other) = delete;

        InputControlSystem(InputControlSystem &&other) = delete;
        InputControlSystem &operator=(InputControlSystem &&other) = delete;

        void Initialize();

        void Update(GLFWwindow *window);

      private:
    };
}; // namespace Prism::Systems