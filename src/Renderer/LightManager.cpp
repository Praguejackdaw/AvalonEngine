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
        PointLight PointLights[256];     // 256 * 48 bytes = 12288 bytes
        SpotLight SpotLights[256];       // 256 * 64 bytes = 16384 bytes
    };

    struct GPULightHeader {
        DirectionalLight DirLight;      // 32 bytes
        uint32_t PointLightCount = 0;    // 4 bytes
        uint32_t SpotLightCount = 0;     // 4 bytes
        float Padding1 = 0.0f;           // 4 bytes
        float Padding2 = 0.0f;           // 4 bytes
    };

    // Static assertions to ensure perfect alignment with OpenGL std430/std140 requirements
    static_assert(sizeof(DirectionalLight) == 32, "DirectionalLight size must be exactly 32 bytes under std140!");
    static_assert(sizeof(PointLight) == 48, "PointLight size must be exactly 48 bytes under std140!");
    static_assert(sizeof(SpotLight) == 64, "SpotLight size must be exactly 64 bytes under std140!");
    static_assert(sizeof(GPULightHeader) == 48, "GPULightHeader size mismatch!");
    static_assert(sizeof(GPULightBufferData) == 28720, "GPULightBufferData alignment mismatch!");

    class GPULightManager : public LightManager {
    public:
        GPULightManager() = default;
        
        ~GPULightManager() override {
            Shutdown();
        }

        void Init() override {
            // DSA: Create Shader Storage Buffer Object instead of UBO to support dynamic array bounds
            glCreateBuffers(1, &m_SSBO);
            
            // Allocate immutable storage for the maximum budget (256 lights) with GL_DYNAMIC_STORAGE_BIT
            glNamedBufferStorage(m_SSBO, sizeof(GPULightBufferData), nullptr, GL_DYNAMIC_STORAGE_BIT);
        }

        void Shutdown() override {
            if (m_SSBO != 0) {
                glDeleteBuffers(1, &m_SSBO);
                m_SSBO = 0;
            }
        }

        void SetDirectionalLight(const DirectionalLight& light) override { m_DirLight = light; }
        
        void AddPointLight(const PointLight& light) override {
            if (m_PointLights.size() < 256) {
                m_PointLights.push_back(light);
            } else {
                std::cerr << "[LightManager Warning] Exceeded maximum PointLight capacity of 256!" << std::endl;
            }
        }

        void AddSpotLight(const SpotLight& light) override {
            if (m_SpotLights.size() < 256) {
                m_SpotLights.push_back(light);
            } else {
                std::cerr << "[LightManager Warning] Exceeded maximum SpotLight capacity of 256!" << std::endl;
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
            // 1. Pack and upload buffer header (Directional Light and active counts)
            GPULightHeader header;
            header.DirLight = m_DirLight;
            header.PointLightCount = static_cast<uint32_t>(m_PointLights.size());
            header.SpotLightCount = static_cast<uint32_t>(m_SpotLights.size());
            header.Padding1 = 0.0f;
            header.Padding2 = 0.0f;

            // Direct State Access: Sync header at offset 0
            glNamedBufferSubData(m_SSBO, 0, sizeof(GPULightHeader), &header);

            // 2. DSA dynamic slice sync: Upload only the active PointLights
            if (!m_PointLights.empty()) {
                glNamedBufferSubData(
                    m_SSBO, 
                    sizeof(GPULightHeader), 
                    m_PointLights.size() * sizeof(PointLight), 
                    m_PointLights.data()
                );
            }

            // 3. DSA dynamic slice sync: Upload only the active SpotLights to their specific structural offset
            if (!m_SpotLights.empty()) {
                uint32_t spotLightsOffset = sizeof(GPULightHeader) + 256 * sizeof(PointLight);
                glNamedBufferSubData(
                    m_SSBO, 
                    spotLightsOffset, 
                    m_SpotLights.size() * sizeof(SpotLight), 
                    m_SpotLights.data()
                );
            }
        }

        void BindLightBuffer(uint32_t bindingPoint) override {
            // Bind as Shader Storage Buffer (SSBO) binding point to slot 1
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_SSBO);
        }

    private:
        uint32_t m_SSBO = 0;
        DirectionalLight m_DirLight;
        std::vector<PointLight> m_PointLights;
        std::vector<SpotLight> m_SpotLights;
    };

    std::shared_ptr<LightManager> LightManager::Create() {
        return std::make_shared<GPULightManager>();
    }

} // namespace Avalon
