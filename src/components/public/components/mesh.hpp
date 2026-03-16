#pragma once

#include <string>

namespace Prism
{
    namespace Resources
    {
        struct MeshResource;
    }

    namespace Components
    {
        struct Mesh
        {
            Resources::MeshResource::ID resourceId = Resources::MeshResource::UNINITIALIZED_ID;
        };
    } // namespace Components
};    // namespace Prism