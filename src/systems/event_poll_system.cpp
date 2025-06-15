#include "systems/event_poll_system.hpp"

namespace Prism::Systems {
    namespace {}; // namespace

    EventPollSystem::EventPollSystem(
        Resources::ContextResources &contextResources)
        : m_contextResources(contextResources) {};

    void EventPollSystem::Initialize() {

    };

    void EventPollSystem::Update() {
        glfwPollEvents();
        m_contextResources.dispatcher.update();
    };
} // namespace Prism::Systems