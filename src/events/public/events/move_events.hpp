#pragma once

#include <utility>

namespace Prism::Events {
    namespace MoveEvents {
        enum class InputAction { None, Pressed };

        enum class Keys { W, A, S, D, SPACE, SHIFT, R, T };

        enum class MouseButton { Left, Right };
    }; // namespace MoveEvents

    using namespace MoveEvents;

    struct KeyPressEvent {
        InputAction action;
        Keys key;
    };

    struct MouseMoveEvent {
        std::pair<double, double> position;
    };

    struct MouseButtonPressEvent {
        InputAction action;
        MouseButton button;
    };
}; // namespace Prism::Events