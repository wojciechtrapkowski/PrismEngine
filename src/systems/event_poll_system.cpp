#include "systems/event_poll_system.hpp"

#include "utils/opengl_debug.hpp"

namespace Prism::Systems {
    namespace {}; // namespace

    EventPollSystem::EventPollSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {};

    void EventPollSystem::Initialize() {

    };

    void EventPollSystem::Update(float deltaTime) {
        GLCheck(glfwPollEvents());
        m_contextResources.dispatcher.update();
    };
} // namespace Prism::Systems