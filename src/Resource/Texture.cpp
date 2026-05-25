#include "Resource/Texture.h"
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <stdexcept>

namespace Avalon {

    Texture::Texture(const std::string& path)
        : m_Path(path) {
        
        int width, height, channels;
        // Flip loaded textures horizontally to match OpenGL UV coordinates
        stbi_set_flip_vertically_on_load(1);

        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4); // Force 4 channels (RGBA)

        if (!data) {
            std::cerr << "Failed to load texture image: " << path << std::endl;
            throw std::runtime_error("Texture resource loading failed!");
        }

        m_Width = static_cast<uint32_t>(width);
        m_Height = static_cast<uint32_t>(height);

        // DSA: Create 2D texture unit
        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

        // Allocate immutable storage (1 mip level for base allocation)
        glTextureStorage2D(m_RendererID, 4, GL_RGBA8, m_Width, m_Height);

        // DSA: Set sampler parameters directly on the texture unit
        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // DSA: Upload texture pixel data
        glTextureSubImage2D(
            m_RendererID,
            0,              // Base mipmap level
            0, 0,           // Texture offset offsets
            m_Width, m_Height,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data
        );

        // DSA: Generate mipmaps automatically on GPU
        glGenerateTextureMipmap(m_RendererID);

        stbi_image_free(data);
    }

    Texture::~Texture() {
        glDeleteTextures(1, &m_RendererID);
    }

    void Texture::Bind(uint32_t slot) const {
        // DSA: Bind texture directly to a specific unit slot without binding it globally
        glBindTextureUnit(slot, m_RendererID);
    }

} // namespace Avalon
