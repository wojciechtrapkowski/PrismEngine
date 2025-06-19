#pragma once

#include "glm/glm.hpp"

namespace Prism::Components {
    struct FpsMotionControl {
        float pitch = 0.0f;
        float yaw = -90.0f;
        glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraForward = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    };
} // namespace Prism::Components