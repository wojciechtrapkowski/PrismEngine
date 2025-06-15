#pragma once

#include <glm/glm.hpp>

namespace Prism::Components {
    struct Transform {
        glm::mat4 transform{1.0f};
    };
}; // namespace Prism::Components