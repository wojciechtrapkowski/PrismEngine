#include "systems/common_uniform_update_system.hpp"

#include "components/fps_camera_control.hpp"
#include "components/fps_motion_control.hpp"
#include "components/tags.hpp"
#include "components/transform.hpp"

#include "resources/common_resource.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "utils/opengl_debug.hpp"

namespace Prism::Systems {
    namespace {
        constexpr glm::vec3 WORLD_FORWARD_VECTOR = {0.0f, 0.0f, -1.0f};
        constexpr glm::vec3 WORLD_RIGHT_VECTOR = {1.0f, 0.0f, 0.0f};
        constexpr glm::vec3 WORLD_UP_VECTOR = {0.0f, 1.0f, 0.0f};
    } // namespace

    CommonUniformUpdateSystem::CommonUniformUpdateSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {
        GLuint commonUbo;
        GLCheck(glGenBuffers(1, &commonUbo));
        GLCheck(glBindBuffer(GL_UNIFORM_BUFFER, commonUbo));

        GLCheck(glBufferData(GL_UNIFORM_BUFFER,
                             sizeof(Resources::CommonResource::ShaderData),
                             nullptr, GL_DYNAMIC_DRAW));

        GLCheck(glBindBufferBase(
            GL_UNIFORM_BUFFER,
            Resources::CommonResource::COMMON_UBO_BINDING_POINT, commonUbo));

        GLCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));

        m_commonResource = Resources::CommonResource{commonUbo};
    };

    void CommonUniformUpdateSystem::Initialize() {

    };

    void CommonUniformUpdateSystem::Update(float deltaTime,
                                           Resources::Scene &scene) {
        auto window = m_contextResources.window.get();

        auto &registry = scene.GetRegistry();

        auto activeCameraView = registry.view<Components::Tags::ActiveCamera>();
        if (activeCameraView.empty()) {
            return;
        }
        auto cameraEntity = activeCameraView.front();

        if (!registry.all_of<Components::FpsCameraControl,
                             Components::FpsMotionControl>(cameraEntity)) {
            return;
        }

        auto &cameraSettings =
            registry.get<Components::FpsCameraControl>(cameraEntity);
        auto &cameraControl =
            registry.get<Components::FpsMotionControl>(cameraEntity);

        auto &cameraPosition = cameraControl.cameraPosition;
        auto &cameraForward = cameraControl.cameraForward;
        auto &cameraRight = cameraControl.cameraRight;
        auto &cameraUp = cameraControl.cameraUp;

        glm::mat4 view = glm::lookAt(cameraPosition,
                                     cameraPosition + cameraForward, cameraUp);

        int width, height;
        GLCheck(glfwGetFramebufferSize(window, &width, &height));
        glViewport(0, 0, width, height);

        float aspectRatio =
            static_cast<float>(width) / static_cast<float>(height);

        glm::mat4 projection =
            glm::perspective(glm::radians(cameraSettings.fov), aspectRatio,
                             cameraSettings.nearPlane, cameraSettings.farPlane);

        Resources::CommonResource::ShaderData shaderData{};
        shaderData.view = std::move(view);
        shaderData.projection = std::move(projection);

        GLCheck(
            glBindBuffer(GL_UNIFORM_BUFFER, m_commonResource.GetUboHandle()));
        GLCheck(glBufferSubData(
            GL_UNIFORM_BUFFER, 0,
            sizeof(Prism::Resources::CommonResource::ShaderData), &shaderData));
        GLCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    };
} // namespace Prism::Systems