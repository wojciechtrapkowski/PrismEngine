#include "resources/shader_resource.hpp"

#include <utility>

namespace Prism::Resources {
    ShaderResource::ShaderResource(GLuint shaderProgram)
        : m_shaderProgram(shaderProgram) {}

    ShaderResource::~ShaderResource() {
        if (m_shaderProgram != 0) {
            glDeleteProgram(m_shaderProgram);
        }
    }

    ShaderResource::ShaderResource(ShaderResource &&other) {
        std::swap(m_shaderProgram, other.m_shaderProgram);
    }

    ShaderResource &ShaderResource::operator=(ShaderResource &&other) {
        std::swap(m_shaderProgram, other.m_shaderProgram);
        return *this;
    }

}; // namespace Prism::Resources