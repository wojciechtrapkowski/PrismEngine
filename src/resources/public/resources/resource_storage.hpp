#pragma once

#include "resources/resource.hpp"

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <optional>
#include <utility>

namespace Prism::Resources
{
    template<class T, class UnderlyingIterator>
    struct TypedIterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::pair<const Resource::ID&, T&>;
        using pointer           = value_type*;
        using reference         = value_type&;

        TypedIterator(UnderlyingIterator begin, UnderlyingIterator end, Resource::TypeID targetTypeId, uint32_t index) :
            _current(begin), _end(end), _targetTypeId(targetTypeId), _index(index)
        {
            _current = std::find_if(_current, _end, [this](const auto& pair) { return pair.second[_index]->GetTypeID() == _targetTypeId; });
        };

        TypedIterator(TypedIterator& other)            = default;
        TypedIterator& operator=(TypedIterator& other) = default;

        TypedIterator(TypedIterator&& other)            = default;
        TypedIterator& operator=(TypedIterator&& other) = default;

        friend bool operator==(const TypedIterator& a, const TypedIterator& b) { return a._current == b._current; };
        friend bool operator!=(const TypedIterator& a, const TypedIterator& b) { return a._current != b._current; };

        // Pre increment operator
        TypedIterator& operator++()
        {
            _current++;
            _current = std::find_if(_current, _end, [this](const auto& pair) { return pair.second[_index]->GetTypeID() == _targetTypeId; });
            return *this;
        }

        // Post increment operator
        TypedIterator operator++(int)
        {
            TypedIterator temp = *this;
            _current++;
            _current = std::find_if(_current, _end, [this](const auto& pair) { return pair.second[_index]->GetTypeID() == _targetTypeId; });
            return temp;
        }

        value_type operator*() const { return {_current->first, static_cast<T&>(*_current->second[_index])}; }

    private:
        UnderlyingIterator _current;
        UnderlyingIterator _end;

        Resource::TypeID _targetTypeId;
        uint32_t         _index;
    };
    struct ResourceStorage
    {
        ResourceStorage()                                  = default;
        ~ResourceStorage()                                 = default;
        ResourceStorage(const ResourceStorage&)            = delete;
        ResourceStorage& operator=(const ResourceStorage&) = delete;

        ResourceStorage(ResourceStorage&&) noexcept            = default;
        ResourceStorage& operator=(ResourceStorage&&) noexcept = default;

        template<typename T>
            requires std::is_base_of<Resources::Resource, T>::value
        void Insert(Resource::ID id, std::unique_ptr<T> resource, size_t index = 0)
        {
            auto& storedResources = _resources[id];
            if (index >= storedResources.size()) {
                storedResources.resize(index + 1);
            }
            resource->SetID(id);
            storedResources[index] = std::move(resource);
        }

        void Delete(Resource::ID id, size_t index = 0)
        {
            auto& storedResources = _resources[id];
            if (index < storedResources.size()) {
                storedResources.erase(storedResources.begin() + index);
            }
        }

        template<typename T>
            requires std::is_base_of<Resource, T>::value
        std::optional<std::reference_wrapper<T>> Get(Resource::ID id, size_t index = 0) const
        {
            auto it = _resources.find(id);
            if (it == _resources.end()) {
                return std::nullopt;
            }
            auto& storedResources = it->second;

            if (index < storedResources.size()) {
                if (storedResources[index] == nullptr) {
                    throw std::runtime_error("It shouldn't be that way - check that!");
                }
                return static_cast<T&>(*storedResources[index]);
            }

            return std::nullopt;
        }

        bool Contains(Resource::ID id, size_t index = 0) const
        {
            auto it = _resources.find(id);
            if (it == _resources.end()) {
                return false;
            }
            auto& storedResources = it->second;

            return index < storedResources.size() && storedResources[index] != nullptr;
        }

        void Clear() { _resources.clear(); }

        template<class T>
        using iterator = TypedIterator<T, std::unordered_map<Resource::ID, std::vector<std::unique_ptr<Resource>>>::iterator>;

        template<class T>
        using const_iterator = TypedIterator<const T, std::unordered_map<Resource::ID, std::vector<std::unique_ptr<Resource>>>::const_iterator>;

        template<class T>
        iterator<T> begin(uint32_t index = 0)
        {
            return iterator<T>(_resources.begin(), _resources.end(), ResourceImpl<T>::TYPE_ID, index);
        }

        template<class T>
        iterator<T> end(uint32_t index = 0)
        {
            return iterator<T>(_resources.end(), _resources.end(), ResourceImpl<T>::TYPE_ID, index);
        }

        template<class T>
        const_iterator<T> begin(uint32_t index = 0) const
        {
            return const_iterator<T>(_resources.cbegin(), _resources.cend(), ResourceImpl<T>::TYPE_ID, index);
        }

        template<class T>
        const_iterator<T> end(uint32_t index = 0) const
        {
            return const_iterator<T>(_resources.cend(), _resources.cend(), ResourceImpl<T>::TYPE_ID, index);
        }

    private:
        std::unordered_map<Resource::ID, std::vector<std::unique_ptr<Resources::Resource>>> _resources = {};
    };

} // namespace Prism::Resources