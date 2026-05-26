#pragma once

#include "Renderer/RenderPass.h"
#include <memory>
#include <glm/glm.hpp>

namespace Avalon {

    class ShadowMap;
    class LightCamera;
    class Shader;
    class FPSCamera;
    class VertexArray;

    /**
     * @brief Renders scene depth details from the perspective of the light source.
     */
    class ShadowPass : public RenderPass {
    public:
        static std::unique_ptr<ShadowPass> Create(uint32_t width = 2048, uint32_t height = 2048);

        ShadowPass(uint32_t width, uint32_t height);
        ~ShadowPass() override;

        void Init() override;
        void Execute(const std::shared_ptr<Scene>& scene) override;

        /**
         * @brief Standard pass execution requiring active lighting contexts
         */
        void ExecuteShadowDepthPass(
            const Scene* scene,
            const glm::vec3& lightDir,
            const std::shared_ptr<FPSCamera>& mainCamera,
            const std::shared_ptr<VertexArray>& customVAO = nullptr,
            const glm::mat4& customTransform = glm::mat4(1.0f)
        );

        const std::unique_ptr<ShadowMap>& GetShadowMap() const { return m_ShadowMap; }
        const std::unique_ptr<LightCamera>& GetLightCamera() const { return m_LightCamera; }

    private:
        std::unique_ptr<ShadowMap> m_ShadowMap;
        std::unique_ptr<LightCamera> m_LightCamera;
        std::shared_ptr<Shader> m_DepthShader;
    };

} // namespace Avalon
