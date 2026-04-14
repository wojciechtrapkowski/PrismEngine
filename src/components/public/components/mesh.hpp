#pragma once

#include <optional>
#include "resources/mesh_resource.hpp"

namespace Prism
{

    namespace Components
    {
        struct Mesh
        {
            Resources::MeshResource::ID resourceId = Resources::MeshResource::UNINITIALIZED_ID;
        };
    } // namespace Components
}; // namespace Prism