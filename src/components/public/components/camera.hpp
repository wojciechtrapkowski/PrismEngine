#pragma once

#include "glm/glm.hpp"

namespace Prism::Components {
    struct alignas(16) Camera {
        glm::mat4 view{};
        glm::mat4 projection{};
    };
} // namespace Prism::Components