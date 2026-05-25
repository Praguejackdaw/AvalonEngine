#pragma once

#include "Core/KeyCodes.h"
#include "Core/MouseButtonCodes.h"
#include <utility>

namespace Avalon {

    class Input {
    public:
        static bool IsKeyPressed(KeyCode keycode);
        static bool IsMouseButtonPressed(MouseButton button);
        static std::pair<float, float> GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();
    };

} // namespace Avalon
