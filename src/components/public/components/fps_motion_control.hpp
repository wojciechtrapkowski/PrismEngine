#pragma once

#include "glm/glm.hpp"

namespace Prism::Components {
    struct FpsMotionControl {
        float pitch = 0.0f;
        float yaw = 0.0f;
    };
} // namespace Prism::Components