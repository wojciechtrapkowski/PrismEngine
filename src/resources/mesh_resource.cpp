#include "resources/mesh_resource.hpp"

namespace Prism::Resources
{
    MeshResource::MeshResource(std::string name, Resources::VkBufferResource<Vertex> vertexBuffer, Resources::VkBufferResource<Index> indexBuffer) :
        _name(std::move(name)), _vertexBuffer(std::move(vertexBuffer)), _indexBuffer(std::move(indexBuffer))
    {}

    MeshResource::MeshResource(MeshResource&& other)
    {
        using std::swap;
        swap(*this, other);
    }

    MeshResource& MeshResource::operator=(MeshResource&& other)
    {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    void swap(MeshResource& lhs, MeshResource& rhs) noexcept
    {
        using std::swap;
        swap(lhs._name, rhs._name);
        swap(lhs._vertexBuffer, rhs._vertexBuffer);
        swap(lhs._indexBuffer, rhs._indexBuffer);
    }
} // namespace Prism::Resources