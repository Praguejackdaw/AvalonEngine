#pragma once

#include "Renderer/RenderPass.h"
#include <memory>
#include <glm/glm.hpp>

namespace Avalon {

    class GBuffer;
    class Shader;
    class FPSCamera;
    class VertexArray;
    class Material;

    class GeometryPass : public RenderPass {
    public:
        static std::unique_ptr<GeometryPass> Create(uint32_t width, uint32_t height);

        GeometryPass(uint32_t width, uint32_t height);
        ~GeometryPass() override;

        void Init() override;
        
        void Execute(const std::shared_ptr<Scene>& scene) override;

        // Pipeline-decoupled high-performance batch rendering methods
        void BeginGeometryPass(const std::shared_ptr<FPSCamera>& mainCamera);
        void RenderMesh(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<Material>& material, const glm::mat4& modelTransform);
        void EndGeometryPass();

        void ExecuteGeometryPass(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<Material>& material, const glm::mat4& modelTransform, const std::shared_ptr<FPSCamera>& mainCamera);

        void Resize(uint32_t width, uint32_t height);
        const std::unique_ptr<GBuffer>& GetGBuffer() const { return m_GBuffer; }

    private:
        std::unique_ptr<GBuffer> m_GBuffer;
        std::shared_ptr<Shader> m_GeometryShader;
    };

} // namespace Avalon
