#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <typeinfo>

namespace Prism::Resources
{
    struct Resource
    {
        using ID     = uint64_t;
        using TypeID = uint64_t;

        virtual ~Resource()              = default;
        virtual TypeID GetTypeID() const = 0;

        // Resource ID is set by the owner of the resource. If the resource is "floating", the ID is std::nullopt.
        virtual std::optional<ID> GetID() const = 0;
        virtual void              SetID(ID id)  = 0;
    };

    template<class T>
    struct UniqueOf
    {
        static uint64_t value() { return static_cast<uint64_t>(typeid(T).hash_code()); }
    };

    template<class T>
    struct ResourceImpl : public Resource
    {
        constexpr static ID        UNINITIALIZED_ID = 0xdeadbeef;
        inline const static TypeID TYPE_ID          = UniqueOf<T>::value();

        TypeID GetTypeID() const override { return TYPE_ID; }

        ResourceImpl()                          = default;
        ResourceImpl(ResourceImpl&&)            = default;
        ResourceImpl& operator=(ResourceImpl&&) = default;

        ResourceImpl(ResourceImpl&)            = delete;
        ResourceImpl& operator=(ResourceImpl&) = delete;

        std::optional<ID> GetID() const override
        {
            if (_id == UNINITIALIZED_ID) {
                return std::nullopt;
            }
            return _id;
        }

        void SetID(ID id) override
        {
            if (_id != UNINITIALIZED_ID) {
                throw std::runtime_error("ID is already set and cannot be changed.");
            }
            _id = id;
        }

    private:
        ID _id = UNINITIALIZED_ID;
    };
} // namespace Prism::Resources