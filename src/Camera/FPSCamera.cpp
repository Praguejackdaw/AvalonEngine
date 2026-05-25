#include "Camera/FPSCamera.h"
#include "Core/Input.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

namespace Avalon {

    FPSCamera::FPSCamera(float fovDeg, float aspectRatio, float nearClip, float farClip)
        : m_Fov(fovDeg), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip) {
        m_Projection = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearClip, m_FarClip);
        UpdateCameraVectors();
    }

    void FPSCamera::SetViewportSize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) return;
        m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
        m_Projection = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearClip, m_FarClip);
    }

    void FPSCamera::OnUpdate(float deltaTime) {
        ProcessKeyboard(deltaTime);
        ProcessMouse();
        
        m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    }

    void FPSCamera::ProcessKeyboard(float deltaTime) {
        float velocity = m_MovementSpeed * deltaTime;

        if (Input::IsKeyPressed(Key::W)) {
            m_Position += m_Front * velocity;
        }
        if (Input::IsKeyPressed(Key::S)) {
            m_Position -= m_Front * velocity;
        }
        if (Input::IsKeyPressed(Key::A)) {
            m_Position -= m_Right * velocity;
        }
        if (Input::IsKeyPressed(Key::D)) {
            m_Position += m_Right * velocity;
        }
        if (Input::IsKeyPressed(Key::Q)) {
            m_Position += m_WorldUp * velocity; // Upward travel
        }
        if (Input::IsKeyPressed(Key::E)) {
            m_Position -= m_WorldUp * velocity; // Downward travel
        }
    }

    void FPSCamera::ProcessMouse() {
        // Industry Best Practice: Only look around when holding Right Mouse Button
        // This keeps the mouse free for ImGui panel interaction!
        if (!Input::IsMouseButtonPressed(Mouse::ButtonRight)) {
            m_FirstMouse = true;
            return;
        }

        float mouseX = Input::GetMouseX();
        float mouseY = Input::GetMouseY();

        if (m_FirstMouse) {
            m_LastMouseX = mouseX;
            m_LastMouseY = mouseY;
            m_FirstMouse = false;
        }

        float xOffset = mouseX - m_LastMouseX;
        float yOffset = m_LastMouseY - mouseY; // Reversed since y-coordinates go from bottom to top

        m_LastMouseX = mouseX;
        m_LastMouseY = mouseY;

        xOffset *= m_MouseSensitivity;
        yOffset *= m_MouseSensitivity;

        m_Yaw   += xOffset;
        m_Pitch += yOffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);

        // Update Front, Right and Up Vectors using the updated Euler angles
        UpdateCameraVectors();
    }

    void FPSCamera::UpdateCameraVectors() {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Front = glm::normalize(front);

        // Re-calculate the Right and Up vectors
        m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down, which results in slower movement.
        m_Up    = glm::normalize(glm::cross(m_Right, m_Front));
    }

} // namespace Avalon
