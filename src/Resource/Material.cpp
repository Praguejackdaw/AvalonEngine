#include "Resource/Material.h"
#include "Resource/Texture.h"
#include "Shader/Shader.h"
#include <glad/glad.h>

namespace Avalon {

    uint32_t BlinnPhongMaterial::s_NextMaterialID = 1;

    std::shared_ptr<BlinnPhongMaterial> BlinnPhongMaterial::Create(
        const std::shared_ptr<Texture>& albedoTex,
        const std::shared_ptr<Texture>& specularTex,
        float shininess
    ) {
        return std::shared_ptr<BlinnPhongMaterial>(new BlinnPhongMaterial(albedoTex, specularTex, shininess));
    }

    BlinnPhongMaterial::BlinnPhongMaterial(
        const std::shared_ptr<Texture>& albedoTex,
        const std::shared_ptr<Texture>& specularTex,
        float shininess
    ) : m_AlbedoTexture(albedoTex), m_SpecularTexture(specularTex), m_Shininess(shininess) {
        m_MaterialID = s_NextMaterialID++;
    }

    void BlinnPhongMaterial::Bind(const std::shared_ptr<Shader>& shader) {
        // Bind Albedo to Slot 0
        if (m_AlbedoTexture) {
            m_AlbedoTexture->Bind(0);
        } else {
            glBindTextureUnit(0, GetFallbackWhiteTexture());
        }

        // Bind Specular to Slot 1
        if (m_SpecularTexture) {
            m_SpecularTexture->Bind(1);
        } else {
            glBindTextureUnit(1, GetFallbackSpecularTexture());
        }

        // Set Shininess Parameter
        shader->SetFloat("u_Material.Shininess", m_Shininess);
    }

    void BlinnPhongMaterial::Unbind() {
        glBindTextureUnit(0, 0);
        glBindTextureUnit(1, 0);
    }

    uint64_t BlinnPhongMaterial::GetSortKey() const {
        // Return 16-bit Material ID in the key range
        return static_cast<uint64_t>(m_MaterialID) & 0xFFFF;
    }

    uint32_t BlinnPhongMaterial::GetFallbackWhiteTexture() {
        static uint32_t textureID = 0;
        if (textureID == 0) {
            glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
            glTextureStorage2D(textureID, 1, GL_RGBA8, 1, 1);
            
            uint32_t pixel = 0xFFFFFFFF; // Solid White
            glTextureSubImage2D(textureID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
            
            glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        return textureID;
    }

    uint32_t BlinnPhongMaterial::GetFallbackSpecularTexture() {
        static uint32_t textureID = 0;
        if (textureID == 0) {
            glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
            glTextureStorage2D(textureID, 1, GL_RGBA8, 1, 1);
            
            uint32_t pixel = 0x000000FF; // Solid Black (No Specularity)
            glTextureSubImage2D(textureID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
            
            glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        return textureID;
    }

    // ==========================================
    // PBRMaterial Implementation
    // ==========================================

    uint32_t PBRMaterial::s_NextMaterialID = 1;

    std::shared_ptr<PBRMaterial> PBRMaterial::Create(
        const std::shared_ptr<Texture>& albedoTex,
        const std::shared_ptr<Texture>& mrTex,
        const std::shared_ptr<Texture>& normalTex,
        const std::shared_ptr<Texture>& aoTex,
        const glm::vec3& albedoFactor,
        float metallicFactor,
        float roughnessFactor,
        float aoFactor
    ) {
        return std::shared_ptr<PBRMaterial>(new PBRMaterial(
            albedoTex, mrTex, normalTex, aoTex,
            albedoFactor, metallicFactor, roughnessFactor, aoFactor
        ));
    }

    PBRMaterial::PBRMaterial(
        const std::shared_ptr<Texture>& albedoTex,
        const std::shared_ptr<Texture>& mrTex,
        const std::shared_ptr<Texture>& normalTex,
        const std::shared_ptr<Texture>& aoTex,
        const glm::vec3& albedoFactor,
        float metallicFactor,
        float roughnessFactor,
        float aoFactor
    ) : m_AlbedoTexture(albedoTex),
        m_MetallicRoughnessTexture(mrTex),
        m_NormalTexture(normalTex),
        m_AOTexture(aoTex) {
        
        m_GPUData.AlbedoFactor = albedoFactor;
        m_GPUData.MetallicFactor = metallicFactor;
        m_GPUData.RoughnessFactor = roughnessFactor;
        m_GPUData.AOFactor = aoFactor;

        m_MaterialID = s_NextMaterialID++;

        // DSA: Allocate a dedicated Uniform Buffer Object for material factors
        glCreateBuffers(1, &m_UBO);
        glNamedBufferStorage(m_UBO, sizeof(GPUMaterialData), &m_GPUData, GL_DYNAMIC_STORAGE_BIT);
    }

    PBRMaterial::~PBRMaterial() {
        if (m_UBO != 0) {
            glDeleteBuffers(1, &m_UBO);
            m_UBO = 0;
        }
    }

    void PBRMaterial::UpdateUBO() {
        if (m_UBO != 0) {
            glNamedBufferSubData(m_UBO, 0, sizeof(GPUMaterialData), &m_GPUData);
        }
    }

    void PBRMaterial::Bind(const std::shared_ptr<Shader>& /*shader*/) {
        // Slot 0: Albedo Map
        if (m_AlbedoTexture) {
            m_AlbedoTexture->Bind(0);
        } else {
            glBindTextureUnit(0, GetFallbackWhiteTexture());
        }

        // Slot 1: Metallic-Roughness Map
        if (m_MetallicRoughnessTexture) {
            m_MetallicRoughnessTexture->Bind(1);
        } else {
            glBindTextureUnit(1, GetFallbackMetallicRoughnessTexture());
        }

        // Slot 2: Normal Map
        if (m_NormalTexture) {
            m_NormalTexture->Bind(2);
        } else {
            glBindTextureUnit(2, GetFallbackNormalTexture());
        }

        // Slot 3: Ambient Occlusion Map
        if (m_AOTexture) {
            m_AOTexture->Bind(3);
        } else {
            glBindTextureUnit(3, GetFallbackAOTexture());
        }

        // DSA: Bind our dynamic factors UBO to global Uniform Buffer Slot 2
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_UBO);
    }

    void PBRMaterial::Unbind() {
        glBindTextureUnit(0, 0);
        glBindTextureUnit(1, 0);
        glBindTextureUnit(2, 0);
        glBindTextureUnit(3, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, 0);
    }

    uint64_t PBRMaterial::GetSortKey() const {
        return static_cast<uint64_t>(m_MaterialID) & 0xFFFF;
    }

    uint32_t PBRMaterial::GetFallbackWhiteTexture() {
        static uint32_t textureID = 0;
        if (textureID == 0) {
            glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
            glTextureStorage2D(textureID, 1, GL_RGBA8, 1, 1);
            
            uint8_t pixels[4] = { 255, 255, 255, 255 }; // Solid White
            glTextureSubImage2D(textureID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            
            glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        return textureID;
    }

    uint32_t PBRMaterial::GetFallbackMetallicRoughnessTexture() {
        static uint32_t textureID = 0;
        if (textureID == 0) {
            glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
            glTextureStorage2D(textureID, 1, GL_RGBA8, 1, 1);
            
            // Standard Dielectric material: Roughness = 1.0 (Green=255), Metallic = 0.0 (Blue=0)
            uint8_t pixels[4] = { 0, 255, 0, 255 }; 
            glTextureSubImage2D(textureID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            
            glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        return textureID;
    }

    uint32_t PBRMaterial::GetFallbackNormalTexture() {
        static uint32_t textureID = 0;
        if (textureID == 0) {
            glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
            glTextureStorage2D(textureID, 1, GL_RGBA8, 1, 1);
            
            // Flat Normal: R=128, G=128, B=255 (Maps to 0.0, 0.0, 1.0 tangent space)
            uint8_t pixels[4] = { 128, 128, 255, 255 };
            glTextureSubImage2D(textureID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            
            glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        return textureID;
    }

    uint32_t PBRMaterial::GetFallbackAOTexture() {
        static uint32_t textureID = 0;
        if (textureID == 0) {
            glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
            glTextureStorage2D(textureID, 1, GL_RGBA8, 1, 1);
            
            uint8_t pixels[4] = { 255, 255, 255, 255 }; // Occlusion Free (AO = 1.0)
            glTextureSubImage2D(textureID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            
            glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        return textureID;
    }

} // namespace Avalon
