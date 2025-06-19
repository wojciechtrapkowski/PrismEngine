#include "resources/common_resource.hpp"

#include <utility>

namespace Prism::Resources {
    CommonResource::CommonResource(GLuint uboHandle) : m_uboHandle(uboHandle) {}

    CommonResource::~CommonResource() {
        if (m_uboHandle != 0) {
            glDeleteBuffers(1, &m_uboHandle);
        }
    }

    CommonResource::CommonResource(CommonResource &&other) {
        std::swap(m_uboHandle, other.m_uboHandle);
    }

    CommonResource &CommonResource::operator=(CommonResource &&other) {
        std::swap(m_uboHandle, other.m_uboHandle);
        return *this;
    }

}; // namespace Prism::Resources