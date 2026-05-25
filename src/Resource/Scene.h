#pragma once

#include "Renderer/Light.h"
#include "Renderer/LightManager.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace Avalon {

    class Model;
    class Shader;
    class ForwardRenderer;

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
        const std::vector<SceneEntity>& GetEntities() const { return m_Entities; }

        // Manage light sources
        void SetDirectionalLight(const DirectionalLight& light) { m_DirLight = light; }
        const DirectionalLight& GetDirectionalLight() const { return m_DirLight; }

        void AddPointLight(const PointLight& light) { m_PointLights.push_back(light); }
        void ClearPointLights() { m_PointLights.clear(); }
        const std::vector<PointLight>& GetPointLights() const { return m_PointLights; }

        void AddSpotLight(const SpotLight& light) { m_SpotLights.push_back(light); }
        void ClearSpotLights() { m_SpotLights.clear(); }
        const std::vector<SpotLight>& GetSpotLights() const { return m_SpotLights; }

        // Draw whole scene using active forward renderer and shader
        void Draw(const std::shared_ptr<ForwardRenderer>& renderer, const std::shared_ptr<Shader>& shader);

    private:
        std::vector<SceneEntity> m_Entities;

        DirectionalLight m_DirLight;
        std::vector<PointLight> m_PointLights;
        std::vector<SpotLight> m_SpotLights;
    };

} // namespace Avalon
