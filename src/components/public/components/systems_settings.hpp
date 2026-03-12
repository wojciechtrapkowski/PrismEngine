#pragma once

namespace Prism::Components
{
    struct MeshDrawingSystemSettings
    {
        enum class MeshDrawingMode
        {
            RASTERIZATION,
            RAYTRACING
        } drawingMode = MeshDrawingMode::RASTERIZATION;
    };
} // namespace Prism::Components