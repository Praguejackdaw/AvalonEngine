#include "Resource/Scene.h"
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

    void Scene::Draw(const std::shared_ptr<Shader>& shader, const std::shared_ptr<FPSCamera>& camera) {
        shader->Bind();

        // 1. Pass Camera uniforms
        shader->SetMat4("u_View", camera->GetViewMatrix());
        shader->SetMat4("u_Projection", camera->GetProjectionMatrix());
        shader->SetFloat3("u_ViewPos", camera->GetPosition());

        // 2. Pass Directional Light uniforms (safely ignored by shaders without these uniforms)
        shader->SetFloat3("u_DirLight.direction", m_DirLight.Direction);
        shader->SetFloat3("u_DirLight.color", m_DirLight.Color);
        shader->SetFloat("u_DirLight.intensity", m_DirLight.Intensity);

        // 3. Pass Point Light uniforms (safely ignored by shaders without these uniforms)
        shader->SetInt("u_NumPointLights", static_cast<int>(m_PointLights.size()));
        for (size_t i = 0; i < m_PointLights.size(); i++) {
            std::string base = "u_PointLights[" + std::to_string(i) + "].";
            shader->SetFloat3(base + "position", m_PointLights[i].Position);
            shader->SetFloat3(base + "color", m_PointLights[i].Color);
            shader->SetFloat(base + "intensity", m_PointLights[i].Intensity);
            shader->SetFloat(base + "constant", m_PointLights[i].Constant);
            shader->SetFloat(base + "linear", m_PointLights[i].Linear);
            shader->SetFloat(base + "quadratic", m_PointLights[i].Quadratic);
        }

        // 4. Pass legacy lighting uniforms for backward compatibility with default.glsl
        // We will map our directional light color and direction to the shader's default pos and color
        shader->SetFloat3("u_LightPos", -m_DirLight.Direction * 5.0f);
        shader->SetFloat3("u_LightColor", m_DirLight.Color * m_DirLight.Intensity);

        // 5. Draw all registered model entities
        for (const auto& entity : m_Entities) {
            shader->SetMat4("u_Model", entity.Transform);
            entity.ModelAsset->Draw(shader);
        }
    }

} // namespace Avalon
