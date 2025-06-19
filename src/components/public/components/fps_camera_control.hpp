#pragma once

namespace Prism::Components {
    struct FpsCameraControl {
        float mouseSensitivity = 0.1f;
        float moveSpeed = 3.0f;
        float fov = 90.0f;
        float nearPlane = 0.01f;
        float farPlane = 1000.0f;
    };
} // namespace Prism::Components.