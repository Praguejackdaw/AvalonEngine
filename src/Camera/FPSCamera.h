#pragma once

#include "Camera/Camera.h"
#include <glm/glm.hpp>

namespace Avalon {

    class FPSCamera : public Camera {
    public:
        FPSCamera(float fovDeg, float aspectRatio, float nearClip, float farClip);
        ~FPSCamera() override = default;

        void OnUpdate(float deltaTime);
        void SetViewportSize(uint32_t width, uint32_t height);

        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        
        glm::vec3 GetPosition() const { return m_Position; }
        void SetPosition(const glm::vec3& position) { m_Position = position; UpdateCameraVectors(); }

        float GetYaw() const { return m_Yaw; }
        float GetPitch() const { return m_Pitch; }

    private:
        void UpdateCameraVectors();
        void ProcessKeyboard(float deltaTime);
        void ProcessMouse();

    private:
        glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 m_Right = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

        // Euler Angles
        float m_Yaw = -90.0f;
        float m_Pitch = 0.0f;

        // Projection settings
        float m_Fov = 45.0f;
        float m_AspectRatio = 16.0f / 9.0f;
        float m_NearClip = 0.1f;
        float m_FarClip = 100.0f;

        // Camera Options
        float m_MovementSpeed = 5.0f;
        float m_MouseSensitivity = 0.1f;

        // Mouse caching state
        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
        bool m_FirstMouse = true;

        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
    };

} // namespace Avalon
