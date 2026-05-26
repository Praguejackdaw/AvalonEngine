#include "Renderer/LightCamera.h"
#include "Camera/FPSCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace Avalon {

    LightCamera::LightCamera(float orthoSize, float zNear, float zFar)
        : m_OrthoSize(orthoSize), m_Near(zNear), m_Far(zFar) {
    }

    void LightCamera::Update(const glm::vec3& lightDir, const std::shared_ptr<FPSCamera>& mainCamera) {
        if (!mainCamera) return;

        // Normalizing light direction safely
        glm::vec3 normalizedLightDir = glm::length(lightDir) > 0.0001f ? glm::normalize(lightDir) : glm::vec3(0.0f, -1.0f, 0.0f);

        // Center the light camera frustum at the main camera's current position to optimize shadow resolution
        glm::vec3 cameraPos = mainCamera->GetPosition();
        
        // Offset light position back along the direction vector to capture objects in front
        glm::vec3 lightPos = cameraPos - normalizedLightDir * (m_Far * 0.5f);
        
        // Fallback up vector to prevent matrix degeneracies when looking straight down or up
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        if (std::abs(glm::dot(normalizedLightDir, up)) > 0.999f) {
            up = glm::vec3(0.0f, 0.0f, 1.0f);
        }
        
        m_View = glm::lookAt(lightPos, cameraPos, up);

        // Create an orthographic projection centered around the camera view
        m_Projection = glm::ortho(
            -m_OrthoSize, m_OrthoSize,
            -m_OrthoSize, m_OrthoSize,
            m_Near, m_Far
        );
    }

} // namespace Avalon
