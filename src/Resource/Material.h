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

        virtual void SetAlbedoTexture(const std::shared_ptr<Texture>& texture) = 0;
        virtual void SetSpecularTexture(const std::shared_ptr<Texture>& texture) = 0;
        virtual void SetShininess(float shininess) = 0;

        virtual const std::shared_ptr<Texture>& GetAlbedoTexture() const = 0;
        virtual const std::shared_ptr<Texture>& GetSpecularTexture() const = 0;
        virtual float GetShininess() const = 0;
        
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

} // namespace Avalon
