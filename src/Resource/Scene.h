#pragma once

#include "Renderer/Light.h"
#include "Camera/FPSCamera.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace Avalon {

    class Model;
    class Shader;

    struct SceneEntity {
        std::shared_ptr<Model> ModelAsset;
        glm::mat4 Transform = glm::mat4(1.0f);
    };

    class Scene {
    public:
        Scene();
        ~Scene() = default;

        // Manage entities
        void AddEntity(const std::shared_ptr<Model>& model, const glm::mat4& transform = glm::mat4(1.0f));
        void ClearEntities() { m_Entities.clear(); }

        // Manage light sources
        void SetDirectionalLight(const DirectionalLight& light) { m_DirLight = light; }
        const DirectionalLight& GetDirectionalLight() const { return m_DirLight; }

        void AddPointLight(const PointLight& light) { m_PointLights.push_back(light); }
        void ClearPointLights() { m_PointLights.clear(); }
        const std::vector<PointLight>& GetPointLights() const { return m_PointLights; }

        // Render whole scene using active shader and camera view projection
        void Draw(const std::shared_ptr<Shader>& shader, const std::shared_ptr<FPSCamera>& camera);

    private:
        std::vector<SceneEntity> m_Entities;

        DirectionalLight m_DirLight;
        std::vector<PointLight> m_PointLights;
    };

} // namespace Avalon
