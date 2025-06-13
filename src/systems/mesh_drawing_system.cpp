#include "systems/mesh_drawing_system.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <filesystem>
#include <iostream>
#include <vector>

namespace Prism::Systems {
    namespace {
        Loaders::ShaderLoader::ShadersSources SHADERS_SOURCES{
            .vertexShader = "basic.vert",
            .fragmentShader = "basic.frag",
        };
    }

    void MeshDrawingSystem::Initialize() {
        Loaders::ShaderLoader shaderLoader;
        auto shaderProgramOpt = shaderLoader(SHADERS_SOURCES);
        if (!shaderProgramOpt) {
            throw std::runtime_error("Couldn't load shaders!");
        }
        m_shaderResource = std::move(*shaderProgramOpt);
    };

    void MeshDrawingSystem::Update() {

    };

    void MeshDrawingSystem::Render(Resources::Scene &scene) {
        glUseProgram(m_shaderResource->GetShaderProgram());

        for (const auto &mesh : scene.GetAllMeshes()) {
            glBindVertexArray(mesh->GetVertexArrayObject());
            glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT,
                           0);
        }
    }
} // namespace Prism::Systems