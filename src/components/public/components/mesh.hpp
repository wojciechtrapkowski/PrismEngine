#pragma once

#include <string>

namespace Prism {
    namespace Resources {
        struct MeshResource;
    }

    namespace Components {
        struct Mesh {
            Resources::MeshResource::ID resourceId;
            std::string name;
        };
    } // namespace Components
}; // namespace Prism