#include "loaders/shader_loader.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>
#include <array>

namespace Prism::Loaders {
    namespace {
        constexpr std::string_view SHADERS_DIR = "shaders/";

        std::optional<GLuint> loadShader(const std::string &filename,
                                         GLenum shaderType) {
            // Read the shader source file

            std::filesystem::path filePath =
                std::string(SHADERS_DIR) + filename;

            std::ifstream file(filePath);
            if (!file.is_open()) {
                std::cout << "ERROR::SHADER::FILE_NOT_FOUND: " << filename
                          << std::endl;
                return std::nullopt;
            }

            // Read file contents into a string
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string shaderCode = buffer.str();

            // Create and compile the shader
            GLuint shader = glCreateShader(shaderType);
            const char *code = shaderCode.c_str();
            glShaderSource(shader, 1, &code, nullptr);
            glCompileShader(shader);

            // Check for compilation errors
            GLint success;
            std::array<char, 512> infoLog;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

            if (!success) {
                glGetShaderInfoLog(shader, infoLog.size(), nullptr,
                                   infoLog.data());
                std::string shaderTypeStr;

                switch (shaderType) {
                case GL_VERTEX_SHADER:
                    shaderTypeStr = "VERTEX";
                    break;
                case GL_FRAGMENT_SHADER:
                    shaderTypeStr = "FRAGMENT";
                    break;
                case GL_GEOMETRY_SHADER:
                    shaderTypeStr = "GEOMETRY";
                    break;
                default:
                    shaderTypeStr = "UNKNOWN";
                }

                std::cout << "ERROR::SHADER::" << shaderTypeStr
                          << "::COMPILATION_FAILED\n"
                          << infoLog.data() << std::endl;
                glDeleteShader(shader);
                return std::nullopt;
            }

            return shader;
        }
    } // namespace

    ShaderLoader::result_type
    ShaderLoader::operator()(const ShadersSources &sources) const {
        try {
            auto vertexShader =
                loadShader(sources.vertexShader, GL_VERTEX_SHADER);
            if (!vertexShader) {
                std::cout << "Current system path: "
                          << std::filesystem::current_path() << std::endl;
                throw std::runtime_error("Couldn't load vertex shader!");
            }

            auto fragmentShader =
                loadShader(sources.fragmentShader, GL_FRAGMENT_SHADER);
            if (!fragmentShader) {
                glDeleteShader(*vertexShader);
                std::cout << "Current system path: "
                          << std::filesystem::current_path() << std::endl;
                throw std::runtime_error("Couldn't load fragment shader!");
            }

            GLuint shaderProgram = glCreateProgram();
            glAttachShader(shaderProgram, *vertexShader);
            glAttachShader(shaderProgram, *fragmentShader);
            glLinkProgram(shaderProgram);

            // Check for linking errors
            GLint success;
            std::array<char, 512> infoLog;
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

            if (!success) {
                glGetProgramInfoLog(shaderProgram, infoLog.size(), nullptr,
                                    infoLog.data());
                std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                          << infoLog.data() << std::endl;
                glDeleteShader(*vertexShader);
                glDeleteShader(*fragmentShader);
                glDeleteProgram(shaderProgram);
                throw std::runtime_error("Shader program linking failed!");
            }

            glDeleteShader(*vertexShader);
            glDeleteShader(*fragmentShader);

            return {std::make_unique<Resources::ShaderResource>(
                std::move(shaderProgram))};
        } catch (const std::exception &e) {
            std::cerr << "Shader compilation error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
} // namespace Prism::Loaders