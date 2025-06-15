#pragma once

namespace Prism {
    namespace Resources {
        struct MeshResource;
    }

    namespace Components {
        struct Mesh {
            Resources::MeshResource::ID resourceId;
        };
    } // namespace Components
}; // namespace Prism