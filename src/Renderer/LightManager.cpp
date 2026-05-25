#include "Renderer/LightManager.h"
#include <glad/glad.h>
#include <iostream>

namespace Avalon {

    struct GPULightBufferData {
        DirectionalLight DirLight;      // 32 bytes
        uint32_t PointLightCount = 0;    // 4 bytes
        uint32_t SpotLightCount = 0;     // 4 bytes
        float Padding1 = 0.0f;           // 4 bytes
        float Padding2 = 0.0f;           // 4 bytes
        PointLight PointLights[32];      // 32 * 48 bytes = 1536 bytes
        SpotLight SpotLights[32];        // 32 * 64 bytes = 2048 bytes
    };

    // Static assertions to ensure perfect alignment with OpenGL std140 requirements
    static_assert(sizeof(DirectionalLight) == 32, "DirectionalLight size must be exactly 32 bytes under std140!");
    static_assert(sizeof(PointLight) == 48, "PointLight size must be exactly 48 bytes under std140!");
    static_assert(sizeof(SpotLight) == 64, "SpotLight size must be exactly 64 bytes under std140!");
    static_assert(sizeof(GPULightBufferData) == 3632, "GPULightBufferData alignment mismatch!");

    class GPULightManager : public LightManager {
    public:
        GPULightManager() = default;
        
        ~GPULightManager() override {
            Shutdown();
        }

        void Init() override {
            // DSA: Create uniform buffer object
            glCreateBuffers(1, &m_UBO);
            
            // Allocate immutable storage with GL_DYNAMIC_STORAGE_BIT for fast updates
            glNamedBufferStorage(m_UBO, sizeof(GPULightBufferData), nullptr, GL_DYNAMIC_STORAGE_BIT);
        }

        void Shutdown() override {
            if (m_UBO != 0) {
                glDeleteBuffers(1, &m_UBO);
                m_UBO = 0;
            }
        }

        void SetDirectionalLight(const DirectionalLight& light) override { m_DirLight = light; }
        
        void AddPointLight(const PointLight& light) override {
            if (m_PointLights.size() < 32) {
                m_PointLights.push_back(light);
            } else {
                std::cerr << "[LightManager Warning] Exceeded maximum PointLight capacity of 32!" << std::endl;
            }
        }

        void AddSpotLight(const SpotLight& light) override {
            if (m_SpotLights.size() < 32) {
                m_SpotLights.push_back(light);
            } else {
                std::cerr << "[LightManager Warning] Exceeded maximum SpotLight capacity of 32!" << std::endl;
            }
        }

        void ClearLights() override {
            m_PointLights.clear();
            m_SpotLights.clear();
        }

        const DirectionalLight& GetDirectionalLight() const override { return m_DirLight; }
        const std::vector<PointLight>& GetPointLights() const override { return m_PointLights; }
        const std::vector<SpotLight>& GetSpotLights() const override { return m_SpotLights; }

        void UpdateGPUBuffer() override {
            GPULightBufferData bufferData;
            bufferData.DirLight = m_DirLight;
            bufferData.PointLightCount = static_cast<uint32_t>(m_PointLights.size());
            bufferData.SpotLightCount = static_cast<uint32_t>(m_SpotLights.size());

            // Zero remaining slots for safety
            std::memset(bufferData.PointLights, 0, sizeof(bufferData.PointLights));
            std::memset(bufferData.SpotLights, 0, sizeof(bufferData.SpotLights));

            for (size_t i = 0; i < m_PointLights.size(); ++i) {
                bufferData.PointLights[i] = m_PointLights[i];
            }

            for (size_t i = 0; i < m_SpotLights.size(); ++i) {
                bufferData.SpotLights[i] = m_SpotLights[i];
            }

            // High performance Direct State Access buffer update
            glNamedBufferSubData(m_UBO, 0, sizeof(GPULightBufferData), &bufferData);
        }

        void BindLightBuffer(uint32_t bindingPoint) override {
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_UBO);
        }

    private:
        uint32_t m_UBO = 0;
        DirectionalLight m_DirLight;
        std::vector<PointLight> m_PointLights;
        std::vector<SpotLight> m_SpotLights;
    };

    std::shared_ptr<LightManager> LightManager::Create() {
        return std::make_shared<GPULightManager>();
    }

} // namespace Avalon
