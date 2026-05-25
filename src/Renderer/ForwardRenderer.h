#pragma once

#include <memory>
#include <glm/glm.hpp>

namespace Avalon {

    class FPSCamera;
    class Shader;
    class VertexArray;
    class Material;
    class LightManager;
    class RenderQueue;

    /**
     * @brief Structure matching standard camera parameters aligned to 16-byte bounds.
     */
    struct alignas(16) GPUCameraBufferData {
        glm::mat4 View;             // 64 bytes
        glm::mat4 Projection;       // 64 bytes
        glm::vec3 CameraPosition;   // 12 bytes
        float Padding = 0.0f;       // 4 bytes
    };

    /**
     * @brief Core Pipeline Orchestrator interface for Modern Forward Rendering.
     */
    class ForwardRenderer {
    public:
        virtual ~ForwardRenderer() = default;

        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        // Frame rendering cycle
        virtual void BeginFrame(const std::shared_ptr<FPSCamera>& camera) = 0;
        virtual void Submit(
            const std::shared_ptr<VertexArray>& vao,
            const std::shared_ptr<Shader>& shader,
            const std::shared_ptr<Material>& material,
            const glm::mat4& transform
        ) = 0;
        virtual void EndFrame() = 0;

        // Configuration queries
        virtual void SetWireframeMode(bool enabled) = 0;
        virtual bool IsWireframeMode() const = 0;
        virtual uint32_t GetDrawCallCount() const = 0;

        virtual const std::shared_ptr<LightManager>& GetLightManager() const = 0;

        static std::shared_ptr<ForwardRenderer> Create();

    protected:
        // Concrete pipeline stages
        virtual void BeginFrameSetup() = 0;
        virtual void Culling() = 0;
        virtual void SortQueue() = 0;
        virtual void SetupLighting() = 0;
        virtual void RenderOpaque() = 0;
        virtual void RenderSkybox() = 0;
        virtual void RenderTransparent() = 0;
        virtual void EndFrameCleanup() = 0;
    };

} // namespace Avalon
