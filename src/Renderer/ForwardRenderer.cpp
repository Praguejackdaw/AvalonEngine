#include "Renderer/ForwardRenderer.h"
#include "Camera/FPSCamera.h"
#include "Shader/Shader.h"
#include "Resource/Material.h"
#include "Renderer/LightManager.h"
#include "Renderer/RenderQueue.h"
#include "Renderer/RenderCommand.h"
#include <glad/glad.h>
#include <iostream>

namespace Avalon {

    static_assert(sizeof(GPUCameraBufferData) == 144, "GPUCameraBufferData size must be exactly 144 bytes under std140!");

    class ModernForwardRenderer : public ForwardRenderer {
    public:
        ModernForwardRenderer() = default;

        ~ModernForwardRenderer() override {
            Shutdown();
        }

        void Init() override {
            // Allocate DSA Uniform Buffer Object for Camera Matrices
            glCreateBuffers(1, &m_CameraUBO);
            glNamedBufferStorage(m_CameraUBO, sizeof(GPUCameraBufferData), nullptr, GL_DYNAMIC_STORAGE_BIT);

            // Initialize LightManager and allocate its GPU buffer
            m_LightManager = LightManager::Create();
            m_LightManager->Init();

            // Allocate Render Queue
            m_RenderQueue = std::make_unique<RenderQueue>();
        }

        void Shutdown() override {
            if (m_CameraUBO != 0) {
                glDeleteBuffers(1, &m_CameraUBO);
                m_CameraUBO = 0;
            }
            if (m_LightManager) {
                m_LightManager->Shutdown();
                m_LightManager.reset();
            }
            m_RenderQueue.reset();
        }

        void BeginFrame(const std::shared_ptr<FPSCamera>& camera) override {
            m_ActiveCamera = camera;
            BeginFrameSetup();
        }

        void Submit(
            const std::shared_ptr<VertexArray>& vao,
            const std::shared_ptr<Shader>& shader,
            const std::shared_ptr<Material>& material,
            const glm::mat4& transform
        ) override {
            if (!shader || !vao) return;

            // Calculate Normal Matrix on CPU to optimize GPU ALU cycles
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

            // Compute 64-bit sort key to minimize state changes
            // Format: [ Bits 63-48: Shader address hash ] [ Bits 47-32: Material ID ] [ Bits 31-0: Depth/Sequence ]
            uint64_t shaderKey = (reinterpret_cast<uintptr_t>(shader.get()) & 0xFFFF) << 48;
            uint64_t materialKey = (material ? (material->GetSortKey() & 0xFFFF) : 0) << 32;
            uint64_t sortKey = shaderKey | materialKey;

            m_RenderQueue->Submit(std::make_unique<MeshDrawCommand>(
                shader, vao, material, transform, normalMatrix, sortKey
            ));
        }

        void EndFrame() override {
            Culling();
            SortQueue();
            SetupLighting();
            
            // Execute drawing stages
            RenderOpaque();
            RenderSkybox();
            RenderTransparent();

            EndFrameCleanup();
        }

        void SetWireframeMode(bool enabled) override {
            m_WireframeMode = enabled;
        }

        bool IsWireframeMode() const override {
            return m_WireframeMode;
        }

        uint32_t GetDrawCallCount() const override {
            return m_RenderQueue ? m_RenderQueue->GetDrawCallCount() : 0;
        }

        const std::shared_ptr<LightManager>& GetLightManager() const override {
            return m_LightManager;
        }

    protected:
        void BeginFrameSetup() override {
            if (!m_ActiveCamera) return;

            // Map Camera details to the UBO
            GPUCameraBufferData cameraData;
            cameraData.View = m_ActiveCamera->GetViewMatrix();
            cameraData.Projection = m_ActiveCamera->GetProjectionMatrix();
            cameraData.CameraPosition = m_ActiveCamera->GetPosition();
            
            glNamedBufferSubData(m_CameraUBO, 0, sizeof(GPUCameraBufferData), &cameraData);

            // Bind Camera UBO globally to slot 0
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_CameraUBO);

            // Bind Light UBO globally to slot 1
            m_LightManager->BindLightBuffer(1);
        }

        void Culling() override {
            // Frustum Culling stub (to be extended in high fidelity pipelines)
        }

        void SortQueue() override {
            if (m_RenderQueue) {
                m_RenderQueue->Sort();
            }
        }

        void SetupLighting() override {
            if (m_LightManager) {
                m_LightManager->UpdateGPUBuffer();
            }
        }

        void RenderOpaque() override {
            if (m_RenderQueue) {
                if (m_WireframeMode) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }

                m_RenderQueue->Execute();

                // Revert polygon mode to solid fill
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }

        void RenderSkybox() override {
            // Skybox rendering pass placeholder
        }

        void RenderTransparent() override {
            // Translucent rendering pass placeholder
        }

        void EndFrameCleanup() override {
            if (m_RenderQueue) {
                m_RenderQueue->Clear();
            }
            if (m_LightManager) {
                m_LightManager->ClearLights();
            }
            m_ActiveCamera.reset();
        }

    private:
        uint32_t m_CameraUBO = 0;
        std::shared_ptr<LightManager> m_LightManager;
        std::unique_ptr<RenderQueue> m_RenderQueue;
        std::shared_ptr<FPSCamera> m_ActiveCamera;
        bool m_WireframeMode = false;
    };

    std::shared_ptr<ForwardRenderer> ForwardRenderer::Create() {
        return std::make_shared<ModernForwardRenderer>();
    }

} // namespace Avalon
