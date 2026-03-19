#pragma once

#include <string>

namespace Prism::Events
{
    struct MeshFileOpenEvent
    {
        std::string filePath;
    };

    struct SceneFileOpenEvent
    {
        std::string filePath;
    };

    struct SceneFileSaveEvent
    {
        std::string filePath;
    };
}; // namespace Prism::Events