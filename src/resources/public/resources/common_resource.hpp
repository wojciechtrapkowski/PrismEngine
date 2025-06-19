#pragma once

#include "resources/resource.hpp"

#include <glm/glm.hpp>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace Prism::Resources {
    struct CommonResource : ResourceImpl<CommonResource> {
        struct alignas(16) ShaderData {
            glm::mat4 projection;
            glm::mat4 view;
        };

        CommonResource() = default;
        CommonResource(GLuint uboHandle);
        ~CommonResource();

        CommonResource(CommonResource &other) = delete;
        CommonResource &operator=(CommonResource &other) = delete;

        CommonResource(CommonResource &&other);
        CommonResource &operator=(CommonResource &&other);

        GLuint GetUboHandle() const { return m_uboHandle; }

        static constexpr GLuint COMMON_UBO_BINDING_POINT = 0;

      private:
        GLuint m_uboHandle;
    };
}; // namespace Prism::Resources