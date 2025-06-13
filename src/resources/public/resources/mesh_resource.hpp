#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vector>

namespace Prism::Resources {
    struct MeshResource {
        struct Vertex {
            glm::vec3 position;
        };

        struct Index {
            uint32_t idx;
        };

        struct MeshDescriptor {
            std::vector<Vertex> vertices;
            std::vector<Index> indices;
        };

        MeshResource(MeshDescriptor meshDescriptor);

        ~MeshResource();

        MeshResource(MeshResource &other) = delete;
        MeshResource &operator=(MeshResource &other) = delete;

        MeshResource(MeshResource &&other);
        MeshResource &operator=(MeshResource &&other);

        GLuint GetVertexArrayObject() const { return m_vertexArrayObject; }

        size_t GetVertexCount() const { return m_vertexCount; }

        size_t GetIndexCount() const { return m_indexCount; }

        friend void swap(MeshResource &lhs, MeshResource &rhs) noexcept;

      private:
        size_t m_vertexCount = 0;
        GLuint m_vertexBuffer = 0;

        size_t m_indexCount = 0;
        GLuint m_indexBuffer = 0;

        GLuint m_vertexArrayObject = 0;
    };

}; // namespace Prism::Resources