#include "systems/common_uniform_update_system.hpp"

#include "components/camera.hpp"
#include "components/fps_camera_control.hpp"
#include "components/tags.hpp"
#include "components/transform.hpp"

#include "resources/common_resource.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "utils/opengl_debug.hpp"

namespace Prism::Systems {
    namespace {} // namespace

    CommonUniformUpdateSystem::CommonUniformUpdateSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {
        GLuint commonUbo;
        GLCheck(glGenBuffers(1, &commonUbo));
        GLCheck(glBindBuffer(GL_UNIFORM_BUFFER, commonUbo));

        GLCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(Components::Camera),
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

        if (!registry.all_of<Components::Camera>(cameraEntity)) {
            return;
        }

        auto &camera = registry.get<Components::Camera>(cameraEntity);

        GLCheck(
            glBindBuffer(GL_UNIFORM_BUFFER, m_commonResource.GetUboHandle()));
        GLCheck(glBufferSubData(GL_UNIFORM_BUFFER, 0,
                                sizeof(Components::Camera), &camera));
        GLCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    };
} // namespace Prism::Systems