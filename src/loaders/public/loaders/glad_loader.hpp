#pragma once

namespace Prism::Loaders {
    struct GladLoader {
        using result_type = void;

        GladLoader() = default;
        ~GladLoader() = default;

        GladLoader(GladLoader &&other) = delete;
        GladLoader &operator=(GladLoader &&) = delete;

        GladLoader(GladLoader &other) = delete;
        GladLoader &operator=(GladLoader &) = delete;

        result_type operator()() const;
    };
}; // namespace Prism::Loaders