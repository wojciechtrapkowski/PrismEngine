#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "resources/resource.hpp"

namespace Prism::Resources {
    struct ShaderResource : ResourceImpl<ShaderResource> {
      public:
        ShaderResource(GLuint shaderProgram);
        ~ShaderResource();

        ShaderResource(ShaderResource &other) = delete;
        ShaderResource &operator=(ShaderResource &other) = delete;

        ShaderResource(ShaderResource &&other);
        ShaderResource &operator=(ShaderResource &&other);

        GLuint GetShaderProgram() const { return m_shaderProgram; }

      private:
        GLuint m_shaderProgram = 0;
    };
}; // namespace Prism::Resources