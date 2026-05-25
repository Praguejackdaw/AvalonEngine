#include "Resource/Scene.h"
#include "Renderer/ForwardRenderer.h"
#include "Resource/Model.h"
#include "Shader/Shader.h"

namespace Avalon {

    Scene::Scene() {
        // Initialize directional light defaults
        m_DirLight.Direction = glm::vec3(-0.2f, -1.0f, -0.3f);
        m_DirLight.Color = glm::vec3(1.0f, 0.95f, 0.9f);
        m_DirLight.Intensity = 1.0f;
    }

    void Scene::AddEntity(const std::shared_ptr<Model>& model, const glm::mat4& transform) {
        SceneEntity entity;
        entity.ModelAsset = model;
        entity.Transform = transform;
        m_Entities.push_back(entity);
    }

    void Scene::Draw(const std::shared_ptr<ForwardRenderer>& renderer, const std::shared_ptr<Shader>& shader) {
        if (!renderer || !shader) return;

        // 1. Register all light sources to the LightManager
        auto& lightManager = renderer->GetLightManager();
        if (lightManager) {
            lightManager->ClearLights();
            lightManager->SetDirectionalLight(m_DirLight);
            
            for (const auto& light : m_PointLights) {
                lightManager->AddPointLight(light);
            }
            
            for (const auto& light : m_SpotLights) {
                lightManager->AddSpotLight(light);
            }
        }

        // 2. Submit all entity mesh elements to the ForwardRenderer queue
        for (const auto& entity : m_Entities) {
            if (entity.ModelAsset) {
                for (const auto& mesh : entity.ModelAsset->GetMeshes()) {
                    renderer->Submit(mesh.GetVertexArray(), shader, mesh.GetMaterial(), entity.Transform);
                }
            }
        }
    }

} // namespace Avalon
