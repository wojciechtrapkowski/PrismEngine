#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <optional>

namespace Prism::Loaders {
    struct ShaderLoader {
        using result_type = std::optional<GLuint>;
        
        ShaderLoader() = default;
        ~ShaderLoader() = default;

        ShaderLoader(ShaderLoader&& other) = delete;
        ShaderLoader& operator=(ShaderLoader&&) = delete;
        
        ShaderLoader(ShaderLoader& other) = delete;
        ShaderLoader& operator=(ShaderLoader&) = delete;

        result_type operator()(const std::string& vertexShaderFilename, 
                              const std::string& fragmentShaderFilename) const;
    };
}