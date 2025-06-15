#pragma once

#include "resources/resource.hpp"

#include <entt/entt.hpp>

namespace Prism::Resources {
    struct ContextResources : ResourceImpl<ContextResources> {
        entt::dispatcher dispatcher;
    };
}; // namespace Prism::Resources