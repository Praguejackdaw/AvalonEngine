#pragma once

#include "Renderer/RenderPass.h"
#include <memory>
#include <glm/glm.hpp>

namespace Avalon {

    class GBuffer;
    class Shader;
    class FPSCamera;
    class VertexArray;

    class LightingPass : public RenderPass {
    public:
        static std::unique_ptr<LightingPass> Create();

        LightingPass();
        ~LightingPass() override;

        void Init() override;
        void Execute(const std::shared_ptr<Scene>& scene) override;

        // Binds the G-buffer textures to the shader and renders the full-screen quad
        void ExecuteLightingPass(
            const std::unique_ptr<GBuffer>& gbuffer, 
            const std::shared_ptr<FPSCamera>& mainCamera,
            uint32_t shadowMapTex,
            const glm::mat4& lightSpaceMatrix,
            float shadowBias,
            int pcfKernelSize
        );

    private:
        std::shared_ptr<Shader> m_DeferredLightingShader;
        uint32_t m_ScreenQuadVAO_ID = 0; // Full-screen quad for lighting calculations
    };

} // namespace Avalon
