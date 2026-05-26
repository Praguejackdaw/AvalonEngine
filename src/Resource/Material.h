#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

namespace Avalon {

    class Texture;
    class Shader;

    /**
     * @brief Abstract base class for all Material assets.
     */
    class Material {
    public:
        virtual ~Material() = default;

        // Binds parameters and texture units directly to shader context
        virtual void Bind(const std::shared_ptr<Shader>& shader) = 0;
        virtual void Unbind() = 0;

        virtual void SetAlbedoTexture(const std::shared_ptr<Texture>& texture) {}
        virtual void SetSpecularTexture(const std::shared_ptr<Texture>& texture) {}
        virtual void SetShininess(float shininess) {}

        virtual const std::shared_ptr<Texture>& GetAlbedoTexture() const { static std::shared_ptr<Texture> empty = nullptr; return empty; }
        virtual const std::shared_ptr<Texture>& GetSpecularTexture() const { static std::shared_ptr<Texture> empty = nullptr; return empty; }
        virtual float GetShininess() const { return 0.0f; }

        // PBR dynamic parameters
        virtual void SetMetallicRoughnessTexture(const std::shared_ptr<Texture>& texture) {}
        virtual void SetNormalTexture(const std::shared_ptr<Texture>& texture) {}
        virtual void SetAOTexture(const std::shared_ptr<Texture>& texture) {}
        
        virtual void SetAlbedoFactor(const glm::vec3& factor) {}
        virtual void SetMetallicFactor(float factor) {}
        virtual void SetRoughnessFactor(float factor) {}
        virtual void SetAOFactor(float factor) {}

        virtual const std::shared_ptr<Texture>& GetMetallicRoughnessTexture() const { static std::shared_ptr<Texture> empty = nullptr; return empty; }
        virtual const std::shared_ptr<Texture>& GetNormalTexture() const { static std::shared_ptr<Texture> empty = nullptr; return empty; }
        virtual const std::shared_ptr<Texture>& GetAOTexture() const { static std::shared_ptr<Texture> empty = nullptr; return empty; }
        
        virtual glm::vec3 GetAlbedoFactor() const { return glm::vec3(1.0f); }
        virtual float GetMetallicFactor() const { return 1.0f; }
        virtual float GetRoughnessFactor() const { return 1.0f; }
        virtual float GetAOFactor() const { return 1.0f; }
        
        virtual uint64_t GetSortKey() const = 0;
    };

    /**
     * @brief Blinn-Phong material implementation utilizing DSA textures.
     */
    class BlinnPhongMaterial : public Material {
    public:
        static std::shared_ptr<BlinnPhongMaterial> Create(
            const std::shared_ptr<Texture>& albedoTex = nullptr,
            const std::shared_ptr<Texture>& specularTex = nullptr,
            float shininess = 32.0f
        );

        ~BlinnPhongMaterial() override = default;

        void Bind(const std::shared_ptr<Shader>& shader) override;
        void Unbind() override;

        void SetAlbedoTexture(const std::shared_ptr<Texture>& texture) override { m_AlbedoTexture = texture; }
        void SetSpecularTexture(const std::shared_ptr<Texture>& texture) override { m_SpecularTexture = texture; }
        void SetShininess(float shininess) override { m_Shininess = shininess; }

        const std::shared_ptr<Texture>& GetAlbedoTexture() const override { return m_AlbedoTexture; }
        const std::shared_ptr<Texture>& GetSpecularTexture() const override { return m_SpecularTexture; }
        float GetShininess() const override { return m_Shininess; }
        
        uint64_t GetSortKey() const override;

    private:
        BlinnPhongMaterial(
            const std::shared_ptr<Texture>& albedoTex,
            const std::shared_ptr<Texture>& specularTex,
            float shininess
        );

        static uint32_t GetFallbackWhiteTexture();
        static uint32_t GetFallbackSpecularTexture();

    private:
        std::shared_ptr<Texture> m_AlbedoTexture;
        std::shared_ptr<Texture> m_SpecularTexture;
        float m_Shininess = 32.0f;
        uint32_t m_MaterialID = 0;

        static uint32_t s_NextMaterialID;
    };

    /**
     * @brief Aligned struct matching std140 layout for dynamic material factor variables.
     */
    struct alignas(16) GPUMaterialData {
        glm::vec3 AlbedoFactor = glm::vec3(1.0f);
        float Padding1 = 0.0f;
        float MetallicFactor = 1.0f;
        float RoughnessFactor = 1.0f;
        float AOFactor = 1.0f;
        float Padding2 = 0.0f;
    };

    /**
     * @brief PBR Cook-Torrance material implementation utilizing DSA textures.
     */
    class PBRMaterial : public Material {
    public:
        static std::shared_ptr<PBRMaterial> Create(
            const std::shared_ptr<Texture>& albedoTex = nullptr,
            const std::shared_ptr<Texture>& mrTex = nullptr,
            const std::shared_ptr<Texture>& normalTex = nullptr,
            const std::shared_ptr<Texture>& aoTex = nullptr,
            const glm::vec3& albedoFactor = glm::vec3(1.0f),
            float metallicFactor = 1.0f,
            float roughnessFactor = 1.0f,
            float aoFactor = 1.0f
        );

        ~PBRMaterial() override;

        void Bind(const std::shared_ptr<Shader>& shader) override;
        void Unbind() override;

        // Base overrides
        void SetAlbedoTexture(const std::shared_ptr<Texture>& texture) override { m_AlbedoTexture = texture; }
        const std::shared_ptr<Texture>& GetAlbedoTexture() const override { return m_AlbedoTexture; }

        // PBR overrides
        void SetMetallicRoughnessTexture(const std::shared_ptr<Texture>& texture) override { m_MetallicRoughnessTexture = texture; }
        void SetNormalTexture(const std::shared_ptr<Texture>& texture) override { m_NormalTexture = texture; }
        void SetAOTexture(const std::shared_ptr<Texture>& texture) override { m_AOTexture = texture; }
        
        void SetAlbedoFactor(const glm::vec3& factor) override { m_GPUData.AlbedoFactor = factor; UpdateUBO(); }
        void SetMetallicFactor(float factor) override { m_GPUData.MetallicFactor = factor; UpdateUBO(); }
        void SetRoughnessFactor(float factor) override { m_GPUData.RoughnessFactor = factor; UpdateUBO(); }
        void SetAOFactor(float factor) override { m_GPUData.AOFactor = factor; UpdateUBO(); }

        const std::shared_ptr<Texture>& GetMetallicRoughnessTexture() const override { return m_MetallicRoughnessTexture; }
        const std::shared_ptr<Texture>& GetNormalTexture() const override { return m_NormalTexture; }
        const std::shared_ptr<Texture>& GetAOTexture() const override { return m_AOTexture; }
        
        glm::vec3 GetAlbedoFactor() const override { return m_GPUData.AlbedoFactor; }
        float GetMetallicFactor() const override { return m_GPUData.MetallicFactor; }
        float GetRoughnessFactor() const override { return m_GPUData.RoughnessFactor; }
        float GetAOFactor() const override { return m_GPUData.AOFactor; }

        uint64_t GetSortKey() const override;

    private:
        PBRMaterial(
            const std::shared_ptr<Texture>& albedoTex,
            const std::shared_ptr<Texture>& mrTex,
            const std::shared_ptr<Texture>& normalTex,
            const std::shared_ptr<Texture>& aoTex,
            const glm::vec3& albedoFactor,
            float metallicFactor,
            float roughnessFactor,
            float aoFactor
        );

        void UpdateUBO();

        static uint32_t GetFallbackWhiteTexture();
        static uint32_t GetFallbackMetallicRoughnessTexture();
        static uint32_t GetFallbackNormalTexture();
        static uint32_t GetFallbackAOTexture();

    private:
        std::shared_ptr<Texture> m_AlbedoTexture;
        std::shared_ptr<Texture> m_MetallicRoughnessTexture;
        std::shared_ptr<Texture> m_NormalTexture;
        std::shared_ptr<Texture> m_AOTexture;

        GPUMaterialData m_GPUData;
        uint32_t m_UBO = 0;

        uint32_t m_MaterialID = 0;
        static uint32_t s_NextMaterialID;
    };

} // namespace Avalon
