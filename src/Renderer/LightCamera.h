#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Avalon {

    class FPSCamera;

    /**
     * @brief Computes dynamic orthographic projections mapping directional light space.
     */
    class LightCamera {
    public:
        LightCamera(float orthoSize = 15.0f, float zNear = -20.0f, float zFar = 30.0f);
        ~LightCamera() = default;

        /**
         * @brief Updates projection bounds tightly around the main camera viewpoint
         *        to maximize depth precision and prevent shadow swimming.
         */
        void Update(const glm::vec3& lightDir, const std::shared_ptr<FPSCamera>& mainCamera);

        glm::mat4 GetViewMatrix() const { return m_View; }
        glm::mat4 GetProjectionMatrix() const { return m_Projection; }
        glm::mat4 GetLightSpaceMatrix() const { return m_Projection * m_View; }

    private:
        glm::mat4 m_View = glm::mat4(1.0f);
        glm::mat4 m_Projection = glm::mat4(1.0f);

        float m_OrthoSize;
        float m_Near;
        float m_Far;
    };

} // namespace Avalon
