#pragma once

#include <glad/glad.h>

#include <optional>
#include <string>

#include "resources/shader_resource.hpp"

namespace Prism::Loaders {
    struct ShaderLoader {
        struct ShadersSources {
            std::string vertexShader;
            std::string fragmentShader;

            // Add optional shaders here
        };

        using result_type =
            std::optional<std::unique_ptr<Resources::ShaderResource>>;

        ShaderLoader() = default;
        ~ShaderLoader() = default;

        ShaderLoader(ShaderLoader &&other) = delete;
        ShaderLoader &operator=(ShaderLoader &&) = delete;

        ShaderLoader(ShaderLoader &other) = delete;
        ShaderLoader &operator=(ShaderLoader &) = delete;

        result_type operator()(const ShadersSources &shaders) const;
    };
} // namespace Prism::Loaders