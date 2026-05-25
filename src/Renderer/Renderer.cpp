#include "Renderer/Renderer.h"
#include "Shader/Shader.h"
#include "Renderer/VertexArray.h"
#include <iostream>

namespace Avalon {

    std::unique_ptr<Renderer::SceneData> Renderer::s_SceneData = std::make_unique<Renderer::SceneData>();

    void Renderer::Init() {
        RenderCommandAPI::Init();
        RenderCommandAPI::SetClearColor(glm::vec4(0.08f, 0.08f, 0.10f, 1.00f));
    }

    void Renderer::Shutdown() {
        // High level renderer resources release
    }

    void Renderer::Update(float deltaTime) {
        // Frame logic updates
    }

    void Renderer::Clear() {
        RenderCommandAPI::Clear();
    }

    void Renderer::BeginScene(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
        s_SceneData->ViewMatrix = viewMatrix;
        s_SceneData->ProjectionMatrix = projectionMatrix;
    }

    void Renderer::EndScene() {
        // Complete frame submittal
    }

    void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform) {
        shader->Bind();
        
        // Pass scene matrices to the shader using modern uniform binds
        shader->SetMat4("u_View", s_SceneData->ViewMatrix);
        shader->SetMat4("u_Projection", s_SceneData->ProjectionMatrix);
        shader->SetMat4("u_Model", transform);

        // Calculate Normal Matrix on CPU to avoid expensive GPU matrix inversion in the Vertex Shader
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
        shader->SetMat3("u_NormalMatrix", normalMatrix);

        RenderCommandAPI::DrawIndexed(vertexArray);
    }

} // namespace Avalon
