#include "resources/mesh_resource.hpp"

namespace Prism::Resources {
    MeshResource::MeshResource(MeshDescriptor meshDescriptor)
        : m_vertexCount(meshDescriptor.vertices.size()),
          m_indexCount(meshDescriptor.indices.size()) {

        // Create vertex array object
        glGenVertexArrays(1, &m_vertexArrayObject);

        // Create vertex & index buffers
        glGenBuffers(1, &m_vertexBuffer);
        glGenBuffers(1, &m_indexBuffer);

        // Bind them & upload data
        glBindVertexArray(m_vertexArrayObject);

        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     meshDescriptor.vertices.size() * sizeof(Vertex),
                     meshDescriptor.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     meshDescriptor.indices.size() * sizeof(Index),
                     meshDescriptor.indices.data(), GL_STATIC_DRAW);

        // Vertex layout
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,        // Attribute location in shader (layout(location = 0))
            3,        // 3 components (x, y, z)
            GL_FLOAT, // Type
            GL_FALSE, // Not normalized
            sizeof(Vertex), // Stride (distance between vertices)
            (void *)offsetof(
                Vertex,
                position) // Offset of 'position' in struct (should be 0)
        );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,        // Attribute location in shader (layout(location = 0))
            3,        // 3 components (x, y, z)
            GL_FLOAT, // Type
            GL_FALSE, // Not normalized
            sizeof(Vertex), // Stride (distance between vertices)
            (void *)offsetof(
                Vertex,
                normal) // Offset of 'position' in struct (should be 0)
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }

    MeshResource::~MeshResource() {
        if (m_vertexArrayObject != 0) {
            glDeleteVertexArrays(1, &m_vertexArrayObject);
        }

        if (m_vertexBuffer != 0) {
            glDeleteBuffers(1, &m_vertexBuffer);
        }

        if (m_indexBuffer != 0) {
            glDeleteBuffers(1, &m_indexBuffer);
        }
    }

    MeshResource::MeshResource(MeshResource &&other) {
        using std::swap;
        swap(*this, other);
    }

    MeshResource &MeshResource::operator=(MeshResource &&other) {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    void swap(MeshResource &lhs, MeshResource &rhs) noexcept {
        using std::swap;
        swap(lhs.m_vertexCount, rhs.m_vertexCount);
        swap(lhs.m_vertexBuffer, rhs.m_vertexBuffer);
        swap(lhs.m_indexCount, rhs.m_indexCount);
        swap(lhs.m_indexBuffer, rhs.m_indexBuffer);
        swap(lhs.m_vertexArrayObject, rhs.m_vertexArrayObject);
    }
} // namespace Prism::Resources