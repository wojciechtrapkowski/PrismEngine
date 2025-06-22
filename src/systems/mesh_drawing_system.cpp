#include "systems/mesh_drawing_system.hpp"

#include "components/mesh.hpp"
#include "components/transform.hpp"

#include "utils/opengl_debug.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

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

    MeshDrawingSystem::MeshDrawingSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {};

    void MeshDrawingSystem::Initialize() {
        Loaders::ShaderLoader shaderLoader;
        auto shaderProgramOpt = shaderLoader(SHADERS_SOURCES);
        if (!shaderProgramOpt) {
            throw std::runtime_error("Couldn't load shaders!");
        }
        m_shaderResource = std::move(*shaderProgramOpt);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    };

    void MeshDrawingSystem::Update(float deltaTime, Resources::Scene &scene) {

    };

    void MeshDrawingSystem::Render(float deltaTime, Resources::Scene &scene) {

        auto &registry = scene.GetRegistry();

        auto meshTransformView =
            registry.view<Components::Mesh, Components::Transform>();

        for (const auto &meshEntity : meshTransformView) {
            glUseProgram(m_shaderResource->GetShaderProgram());

            const auto &meshResourceId =
                meshTransformView.get<Components::Mesh>(meshEntity).resourceId;

            const auto &transform =
                meshTransformView.get<Components::Transform>(meshEntity)
                    .transform;

            const auto &meshOpt = scene.GetMesh(meshResourceId);
            if (!meshOpt) {
                continue;
            }
            const auto &mesh = *meshOpt;

            GLint modelLoc = glGetUniformLocation(
                m_shaderResource->GetShaderProgram(), "model");

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
                               glm::value_ptr(transform));

            GLCheck(glBindVertexArray(mesh.get().GetVertexArrayObject()));
            GLCheck(glDrawElements(GL_TRIANGLES, mesh.get().GetIndexCount(),
                                   GL_UNSIGNED_INT, 0));
        }
    }
} // namespace Prism::Systems