#pragma once

#include <glm/glm.hpp>

#include "vulkan/vulkan.h"

#include <vector>

#include "resources/resource.hpp"
#include "resources/vulkan/vk_buffer_resource.hpp"

namespace Prism::Resources
{
    struct MeshResource : ResourceImpl<MeshResource>
    {
        static inline const VkFormat    VERTEX_TYPE = VK_FORMAT_R32G32B32_SFLOAT;
        static inline const VkIndexType INDEX_TYPE  = VK_INDEX_TYPE_UINT32;

        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 textureUV;
        };

        struct Index
        {
            uint32_t idx;
        };

        MeshResource(Resources::VkBufferResource<Vertex> vertexBuffer, Resources::VkBufferResource<Index> indexBuffer);

        ~MeshResource() = default;

        MeshResource(MeshResource& other)            = delete;
        MeshResource& operator=(MeshResource& other) = delete;

        MeshResource(MeshResource&& other);
        MeshResource& operator=(MeshResource&& other);

        Resources::VkBufferResource<Vertex>& GetVertexBuffer() { return vertexBuffer; }

        Resources::VkBufferResource<Index>& GetIndexBuffer() { return indexBuffer; }

        VkFormat    GetVertexType() const { return VERTEX_TYPE; }
        VkIndexType GetIndexType() const { return INDEX_TYPE; }

        friend void swap(MeshResource& lhs, MeshResource& rhs) noexcept;

    private:
        Resources::VkBufferResource<Vertex> vertexBuffer = {};
        Resources::VkBufferResource<Index>  indexBuffer  = {};
    };

}; // namespace Prism::Resources