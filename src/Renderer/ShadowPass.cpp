#include "Renderer/ShadowPass.h"
#include "Renderer/ShadowMap.h"
#include "Renderer/LightCamera.h"
#include "Renderer/RenderStateManager.h"
#include "Renderer/RenderCommand.h"
#include "Shader/Shader.h"
#include "Resource/Scene.h"
#include "Resource/Model.h"
#include <glad/glad.h>
#include <iostream>

namespace Avalon {

    std::unique_ptr<ShadowPass> ShadowPass::Create(uint32_t width, uint32_t height) {
        return std::make_unique<ShadowPass>(width, height);
    }

    ShadowPass::ShadowPass(uint32_t width, uint32_t height) {
        m_ShadowMap = ShadowMap::Create(width, height, ShadowType::Directional);
        m_LightCamera = std::make_unique<LightCamera>(15.0f, -20.0f, 30.0f); // Default orthographic bounds centered at player view
    }

    ShadowPass::~ShadowPass() {
        m_ShadowMap.reset();
        m_LightCamera.reset();
        m_DepthShader.reset();
    }

    void ShadowPass::Init() {
        // Allocate the high-performance dedicated Depth Shader
        m_DepthShader = std::make_shared<Shader>("assets/shaders/shadowDepth.glsl");
    }

    void ShadowPass::Execute(const std::shared_ptr<Scene>& scene) {
        // Fallback interface redirecting to default directional light setup
        if (!scene) return;
        ExecuteShadowDepthPass(scene.get(), scene->GetDirectionalLight().Direction, nullptr);
    }

    void ShadowPass::ExecuteShadowDepthPass(
        const Scene* scene,
        const glm::vec3& lightDir,
        const std::shared_ptr<FPSCamera>& mainCamera,
        const std::shared_ptr<VertexArray>& customVAO,
        const glm::mat4& customTransform
    ) {
        if (!m_DepthShader) return;

        // 1. Calculate dynamic tight-frustum light matrices centered around camera position
        m_LightCamera->Update(lightDir, mainCamera);

        // 2. Configure high-precision state overrides
        RenderStateManager::SetDepthTest(true);
        RenderStateManager::SetDepthFunc(GL_LESS);
        
        // Front-Face Culling: Rendering only back faces of the caster geometry.
        // This is a premium graphics trick pushing depth acne naturally inside the volume!
        RenderStateManager::SetCullFace(true);
        RenderStateManager::SetCullFaceMode(GL_FRONT);
        RenderStateManager::SetScissorTest(false);

        // 3. Bind the depth-only Framebuffer FBO
        m_ShadowMap->Bind();
        RenderStateManager::SetViewport(0, 0, m_ShadowMap->GetWidth(), m_ShadowMap->GetHeight());
        
        // Clear depth attachment only (no color attachments exist)
        glClear(GL_DEPTH_BUFFER_BIT);

        // 4. Bind Depth Shader and write Light-Space projection matrices
        m_DepthShader->Bind();
        m_DepthShader->SetMat4("u_LightSpaceMatrix", m_LightCamera->GetLightSpaceMatrix());

        // 5. Draw all opaque scene entities directly (bypassing redundant material & texture bindings)
        if (scene) {
            for (const auto& entity : scene->GetEntities()) {
                if (!entity.ModelAsset) continue;
                
                m_DepthShader->SetMat4("u_Model", entity.Transform);
                
                for (const auto& mesh : entity.ModelAsset->GetMeshes()) {
                    RenderCommandAPI::DrawIndexed(mesh.GetVertexArray());
                }
            }
        }

        // Draw custom procedural VAO if supplied
        if (customVAO) {
            m_DepthShader->SetMat4("u_Model", customTransform);
            RenderCommandAPI::DrawIndexed(customVAO);
        }

        // 6. Clean up FBO binding (ForwardRenderer will reset back to default 3D states lazily)
        m_ShadowMap->Unbind();
    }

} // namespace Avalon
