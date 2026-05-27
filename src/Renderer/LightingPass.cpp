#include "Renderer/LightingPass.h"
#include "Renderer/GBuffer.h"
#include "Renderer/RenderStateManager.h"
#include "Shader/Shader.h"
#include "Camera/FPSCamera.h"
#include <glad/glad.h>

namespace Avalon {

    std::unique_ptr<LightingPass> LightingPass::Create() {
        return std::make_unique<LightingPass>();
    }

    LightingPass::LightingPass() {
        // Create an empty VAO for the full-screen quad trick
        glCreateVertexArrays(1, &m_ScreenQuadVAO_ID);
    }

    LightingPass::~LightingPass() {
        if (m_ScreenQuadVAO_ID != 0) {
            glDeleteVertexArrays(1, &m_ScreenQuadVAO_ID);
        }
    }

    void LightingPass::Init() {
        m_DeferredLightingShader = std::make_shared<Shader>("assets/shaders/deferred_lighting.glsl");
        
        // Pre-bind texture samplers
        m_DeferredLightingShader->Bind();
        m_DeferredLightingShader->SetInt("gDepth", 0); // Bind G-Buffer depth instead of Position
        m_DeferredLightingShader->SetInt("gNormal", 1);
        m_DeferredLightingShader->SetInt("gAlbedo", 2);
        m_DeferredLightingShader->SetInt("gMetallicRoughness", 3);
        m_DeferredLightingShader->SetInt("u_ShadowMap", 4);
    }

    void LightingPass::Execute(const std::shared_ptr<Scene>& scene) {
        // Fallback interface
    }

    void LightingPass::ExecuteLightingPass(
        const std::unique_ptr<GBuffer>& gbuffer, 
        const std::shared_ptr<FPSCamera>& mainCamera,
        uint32_t shadowMapTex,
        const glm::mat4& lightSpaceMatrix,
        float shadowBias,
        int pcfKernelSize
    ) {
        if (!m_DeferredLightingShader || !gbuffer || !mainCamera) return;

        // 1. Setup states for full-screen quad
        RenderStateManager::SetDepthTest(false);
        RenderStateManager::SetCullFace(false);
        RenderStateManager::SetBlend(false);

        // 2. Bind the shader
        m_DeferredLightingShader->Bind();
        m_DeferredLightingShader->SetFloat3("u_ViewPos", mainCamera->GetPosition());
        m_DeferredLightingShader->SetMat4("u_LightSpaceMatrix", lightSpaceMatrix);
        m_DeferredLightingShader->SetFloat("u_ShadowBiasConstant", shadowBias);
        m_DeferredLightingShader->SetInt("u_PCFKernelSize", pcfKernelSize);
        
        // Reconstruct position: pass inverse View-Projection matrix
        glm::mat4 invVP = glm::inverse(mainCamera->GetProjectionMatrix() * mainCamera->GetViewMatrix());
        m_DeferredLightingShader->SetMat4("u_InverseVP", invVP);

        // 3. Bind G-Buffer textures
        glBindTextureUnit(0, gbuffer->GetDepthTexture()); // Bind Depth Texture to slot 0
        glBindTextureUnit(1, gbuffer->GetNormalTexture());
        glBindTextureUnit(2, gbuffer->GetAlbedoTexture());
        glBindTextureUnit(3, gbuffer->GetMetallicRoughnessTexture());
        glBindTextureUnit(4, shadowMapTex);

        // 4. Draw the full-screen quad (using empty VAO + 3 vertices trick)
        glBindVertexArray(m_ScreenQuadVAO_ID);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // Reset state
        RenderStateManager::SetDepthTest(true);
    }

} // namespace Avalon
