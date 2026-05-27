#include "Renderer/GBuffer.h"
#include <glad/glad.h>
#include <stdexcept>
#include <iostream>

namespace Avalon {

    std::unique_ptr<GBuffer> GBuffer::Create(uint32_t width, uint32_t height) {
        return std::make_unique<GBuffer>(width, height);
    }

    GBuffer::GBuffer(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height) {
        Invalidate();
    }

    GBuffer::~GBuffer() {
        Cleanup();
    }

    void GBuffer::Cleanup() {
        if (m_FBO != 0) {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }
        if (m_NormalTex != 0) glDeleteTextures(1, &m_NormalTex);
        if (m_AlbedoTex != 0) glDeleteTextures(1, &m_AlbedoTex);
        if (m_MetallicRoughnessTex != 0) glDeleteTextures(1, &m_MetallicRoughnessTex);
        if (m_DepthTex != 0) glDeleteTextures(1, &m_DepthTex);
        
        m_NormalTex = m_AlbedoTex = m_MetallicRoughnessTex = m_DepthTex = 0;
    }

    void GBuffer::Resize(uint32_t width, uint32_t height) {
        if (m_Width == width && m_Height == height) return;
        m_Width = width;
        m_Height = height;
        Invalidate();
    }

    void GBuffer::Invalidate() {
        Cleanup();

        glCreateFramebuffers(1, &m_FBO);

        // Helper lambda for creating 2D textures via DSA
        auto createTex = [this](uint32_t& texID, GLenum internalFormat) {
            glCreateTextures(GL_TEXTURE_2D, 1, &texID);
            glTextureStorage2D(texID, 1, internalFormat, m_Width, m_Height);
            glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(texID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(texID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        };

        // 1. Normal buffer (RGBA16F for high precision normal mapping)
        createTex(m_NormalTex, GL_RGBA16F);
        glNamedFramebufferTexture(m_FBO, GL_COLOR_ATTACHMENT0, m_NormalTex, 0);

        // 2. Albedo buffer (RGBA8)
        createTex(m_AlbedoTex, GL_RGBA8);
        glNamedFramebufferTexture(m_FBO, GL_COLOR_ATTACHMENT1, m_AlbedoTex, 0);

        // 3. Metallic/Roughness buffer (RG16F)
        createTex(m_MetallicRoughnessTex, GL_RG16F);
        glNamedFramebufferTexture(m_FBO, GL_COLOR_ATTACHMENT2, m_MetallicRoughnessTex, 0);

        // 4. Depth buffer (Immutable 32F Depth attachment)
        glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthTex);
        glTextureStorage2D(m_DepthTex, 1, GL_DEPTH_COMPONENT32F, m_Width, m_Height);
        glTextureParameteri(m_DepthTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_DepthTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(m_DepthTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_DepthTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Channel Swizzle Broadcast: Map single-channel depth (Red) to RGB for beautiful grayscale preview
        int swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
        glTextureParameteriv(m_DepthTex, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

        glNamedFramebufferTexture(m_FBO, GL_DEPTH_ATTACHMENT, m_DepthTex, 0);

        // Specify which color attachments we'll use for rendering (3 color channels total)
        uint32_t attachments[3] = { 
            GL_COLOR_ATTACHMENT0, 
            GL_COLOR_ATTACHMENT1, 
            GL_COLOR_ATTACHMENT2 
        };
        glNamedFramebufferDrawBuffers(m_FBO, 3, attachments);

        if (glCheckNamedFramebufferStatus(m_FBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("GBuffer Framebuffer is incomplete!");
        }
    }

    void GBuffer::Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    }

    void GBuffer::Unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

} // namespace Avalon
