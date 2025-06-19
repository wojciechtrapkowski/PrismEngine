#pragma once

#include <glm/glm.hpp>

namespace Prism::Components {
    struct Transform {
        glm::mat4 transform = glm::mat4(1.0f);
    };
}; // namespace Prism::Components