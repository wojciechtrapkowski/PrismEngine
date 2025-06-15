#pragma once

#include "resources/mesh_resource.hpp"

namespace Prism::Loaders {
    struct MeshLoader {
        using result_type =
            std::optional<std::unique_ptr<Resources::MeshResource>>;

        MeshLoader() = default;
        ~MeshLoader() = default;

        MeshLoader(MeshLoader &&other) = default;
        MeshLoader &operator=(MeshLoader &&) = default;

        MeshLoader(MeshLoader &other) = delete;
        MeshLoader &operator=(MeshLoader &) = delete;

        result_type operator()(const std::string &path) const;
    };
}; // namespace Prism::Loaders