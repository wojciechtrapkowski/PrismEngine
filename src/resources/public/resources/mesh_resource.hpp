#pragma once

#include <glm/glm.hpp>

#include "volk/volk.h"

#include <vector>

#include "resources/resource.hpp"
#include "resources/vulkan/vk_buffer_resource.hpp"
#include "resources/vulkan/vk_texture_resource.hpp"

namespace Prism::Resources
{

    // Thanks to such approach we eliminate problem with sub allocations and their life-cycle management.
    // When main buffer dies this buffer allocation becomes invalid. It's caller's responsibility to check if the buffer exists.
    // When allocation dies it's a bigger problem. We assume that system that creates a parent buffer performs defragmentation sometimes.
    struct BufferAllocation
    {
        Resources::VkBufferResource<>::ID bufferId = {};
        uint32_t                          offset   = 0;
        uint32_t                          size     = 0;
    };

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

        MeshResource(
            std::string                                 name,
            Resources::VkBufferResource<Vertex>         vertexBuffer,
            Resources::VkBufferResource<Index>          indexBuffer,
            std::optional<Resources::VkTextureResource> texture = std::nullopt);

        ~MeshResource() = default;

        MeshResource(MeshResource& other)            = delete;
        MeshResource& operator=(MeshResource& other) = delete;

        MeshResource(MeshResource&& other);
        MeshResource& operator=(MeshResource&& other);

        Resources::VkBufferResource<Vertex>& GetVertexBuffer() { return _vertexBuffer; }

        Resources::VkBufferResource<Index>& GetIndexBuffer() { return _indexBuffer; }

        std::optional<Resources::VkTextureResource>& GetTexture() { return _texture; }

        const std::string& GetName() const { return _name; }

        VkFormat    GetVertexType() const { return VERTEX_TYPE; }
        VkIndexType GetIndexType() const { return INDEX_TYPE; }

        friend void swap(MeshResource& lhs, MeshResource& rhs) noexcept;

    private:
        std::string _name = "None";

        Resources::VkBufferResource<Vertex>         _vertexBuffer = {};
        Resources::VkBufferResource<Index>          _indexBuffer  = {};
        std::optional<Resources::VkTextureResource> _texture      = std::nullopt;

        // BufferAllocation                _vertexBuffer = {};
        // BufferAllocation                _indexBuffer  = {};
        // std::optional<BufferAllocation> _texture      = {};
    };

}; // namespace Prism::Resources