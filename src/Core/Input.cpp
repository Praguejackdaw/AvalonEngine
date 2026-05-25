#include "Core/Input.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include <GLFW/glfw3.h>

namespace Avalon {

    bool Input::IsKeyPressed(KeyCode keycode) {
        auto* window = Application::Get().GetWindow().GetNativeWindow();
        int state = glfwGetKey(window, static_cast<int>(keycode));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(MouseButton button) {
        auto* window = Application::Get().GetWindow().GetNativeWindow();
        int state = glfwGetMouseButton(window, static_cast<int>(button));
        return state == GLFW_PRESS;
    }

    std::pair<float, float> Input::GetMousePosition() {
        auto* window = Application::Get().GetWindow().GetNativeWindow();
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return { static_cast<float>(xpos), static_cast<float>(ypos) };
    }

    float Input::GetMouseX() {
        return GetMousePosition().first;
    }

    float Input::GetMouseY() {
        return GetMousePosition().second;
    }

} // namespace Avalon
