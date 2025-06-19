#include "systems/mesh_drawing_system.hpp"

#include "components/mesh.hpp"

#include "utils/opengl_debug.hpp"

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

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDisable(GL_CULL_FACE);
    };

    void MeshDrawingSystem::Update() {

    };

    void MeshDrawingSystem::Render(Resources::Scene &scene) {
        glUseProgram(m_shaderResource->GetShaderProgram());

        auto &registry = scene.GetRegistry();

        auto meshView = registry.view<Components::Mesh>();

        for (const auto &meshEntity : meshView) {
            const auto &meshResourceId =
                meshView.get<Components::Mesh>(meshEntity).resourceId;
            const auto &meshOpt = scene.GetMesh(meshResourceId);
            if (!meshOpt) {
                continue;
            }
            const auto &mesh = *meshOpt;

            GLCheck(glBindVertexArray(mesh.get().GetVertexArrayObject()));
            GLCheck(glDrawElements(GL_TRIANGLES, mesh.get().GetIndexCount(),
                                   GL_UNSIGNED_INT, 0));
        }
    }
} // namespace Prism::Systems