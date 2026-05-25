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
        shader->SetInt("u_Material.Albedo", 0);

        // Bind Specular to Slot 1
        if (m_SpecularTexture) {
            m_SpecularTexture->Bind(1);
        } else {
            glBindTextureUnit(1, GetFallbackSpecularTexture());
        }
        shader->SetInt("u_Material.Specular", 1);

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

} // namespace Avalon
