#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "resources/context_resources.hpp"

namespace Prism::Systems {
    class EventPollSystem {
      public:
        EventPollSystem(Resources::ContextResources &contextResources);
        ~EventPollSystem() = default;

        EventPollSystem(EventPollSystem &other) = delete;
        EventPollSystem &operator=(EventPollSystem &other) = delete;

        EventPollSystem(EventPollSystem &&other) = delete;
        EventPollSystem &operator=(EventPollSystem &&other) = delete;

        void Initialize();

        void Update();

      private:
        Resources::ContextResources &m_contextResources;
    };
}; // namespace Prism::Systems