#pragma once

#include <cstddef>
#include <memory>

namespace Prism::Resources {
    struct Resource {
        using ID = uint64_t;
        using TypeID = uint64_t;

        virtual ~Resource() = default;
        virtual TypeID GetTypeID() const = 0;
    };

    template <class T> struct UniqueOf {

        static uint64_t value() {
            return reinterpret_cast<uint64_t>(
                reinterpret_cast<void *>(MAGIC_KEY));
        };

      private:
        constexpr const static auto MAGIC_KEY = +[]() {};
    };

    template <class T> struct ResourceImpl : public Resource {
        inline const static TypeID TYPE_ID = UniqueOf<T>::value();

        TypeID GetTypeID() const override { return TYPE_ID; }

        ResourceImpl() = default;
        ResourceImpl(ResourceImpl &&) = default;
        ResourceImpl &operator=(ResourceImpl &&) = default;

        ResourceImpl(ResourceImpl &) = delete;
        ResourceImpl &operator=(ResourceImpl &) = delete;
    };

} // namespace Prism::Resources