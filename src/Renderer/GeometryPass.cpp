#include "Renderer/GeometryPass.h"
#include "Renderer/GBuffer.h"
#include "Renderer/RenderStateManager.h"
#include "Renderer/RenderCommand.h"
#include "Shader/Shader.h"
#include "Resource/Scene.h"
#include "Resource/Model.h"
#include "Camera/FPSCamera.h"
#include "Resource/Material.h"
#include <glad/glad.h>

namespace Avalon {

    std::unique_ptr<GeometryPass> GeometryPass::Create(uint32_t width, uint32_t height) {
        return std::make_unique<GeometryPass>(width, height);
    }

    GeometryPass::GeometryPass(uint32_t width, uint32_t height) {
        m_GBuffer = GBuffer::Create(width, height);
    }

    GeometryPass::~GeometryPass() {
        m_GBuffer.reset();
        m_GeometryShader.reset();
    }

    void GeometryPass::Init() {
        m_GeometryShader = std::make_shared<Shader>("assets/shaders/geometry.glsl");
    }

    void GeometryPass::Execute(const std::shared_ptr<Scene>& scene) {
        // Fallback interface
    }

    void GeometryPass::BeginGeometryPass(const std::shared_ptr<FPSCamera>& mainCamera) {
        if (!m_GeometryShader || !mainCamera) return;

        // 1. Configure state for opaque geometry rendering
        RenderStateManager::SetDepthTest(true);
        RenderStateManager::SetDepthFunc(GL_LESS);
        RenderStateManager::SetCullFace(true);
        RenderStateManager::SetCullFaceMode(GL_BACK);
        // CRITICAL: Disable blending! We are writing raw data to the G-Buffer
        RenderStateManager::SetBlend(false);

        // 2. Bind G-Buffer and set viewport
        m_GBuffer->Bind();
        RenderStateManager::SetViewport(0, 0, m_GBuffer->GetWidth(), m_GBuffer->GetHeight());

        // 3. Clear G-Buffer attachments (Normal, Albedo, MR, and Depth)
        // Clear color to black, and depth to 1.0
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 4. Bind geometry shader
        m_GeometryShader->Bind();
    }

    void GeometryPass::RenderMesh(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<Material>& material, const glm::mat4& modelTransform) {
        if (!m_GeometryShader || !vao || !material) return;

        m_GeometryShader->SetMat4("u_Model", modelTransform);

        // Calculate normal matrix
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelTransform)));
        m_GeometryShader->SetMat3("u_NormalMatrix", normalMatrix);

        material->Bind(m_GeometryShader);
        RenderCommandAPI::DrawIndexed(vao);
    }

    void GeometryPass::EndGeometryPass() {
        if (m_GBuffer) {
            m_GBuffer->Unbind();
        }
    }

    void GeometryPass::ExecuteGeometryPass(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<Material>& material, const glm::mat4& modelTransform, const std::shared_ptr<FPSCamera>& mainCamera) {
        BeginGeometryPass(mainCamera);
        RenderMesh(vao, material, modelTransform);
        EndGeometryPass();
    }

    void GeometryPass::Resize(uint32_t width, uint32_t height) {
        if (m_GBuffer) {
            m_GBuffer->Resize(width, height);
        }
    }

} // namespace Avalon
